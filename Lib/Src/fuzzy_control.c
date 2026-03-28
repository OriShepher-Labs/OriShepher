#include "fuzzy_control.h"

// 原始距离参数
float dist_left, dist_middle, dist_right;

// 距离区间 常量
const float RANGE_NEAR_START       = 30.0f;    // 近 起始
const float RANGE_MEDIUM_START      = 50.0f;    // 中 起始
const float RANGE_NEAR_END         = 70.0f;    // 近 结束
const float RANGE_MEDIUM_MEDIUM     = 80.0f;    // 中 中间
const float RANGE_FAR_START         = 90.0f;    // 远 起始
const float RANGE_MEDIUM_END        = 110.0f;   // 中 结束
const float RANGE_FAR_END           = 150.0f;   // 远 结束

// 隶属度/权重  (0.0 ~ 1.0)
typedef struct {
    float near;     // 近
    float medium;   // 中
    float far;      // 远
} Fuzzy_Weight;
Fuzzy_Weight fuzzy_leftWeight, fuzzy_middleWeight, fuzzy_rightWeight;

// 输出挡位 常量
const float LB = -3.0f; // 左大转   negative big
const float LS = -1.5f; // 左小转   negative small
const float FW =  0.0f; // 直行     forward
const float RS =  1.5f; // 右小转   right small
const float RB =  3.0f; // 右大转   right big

// 规则库 [left][middle][right]
static const float rule_table[3][3][3] = {
    {// 左近 [0][][]
        {// 中近 [0][0][]
            LB, LB, LB,  // 右: 近,中,远 -> 左大转
        },
        {// 中中 [0][1][]
            LB, LS, LS,  // 右: 近,中,远 -> 左大转/左小转/左小转
        },
        {// 中远 [0][2][]
            LB, LS, FW,  // 右: 近,中,远 -> 左大转/左小转/直行
        }
    },
    {// 左中 [1][][]
        {// 中近 [1][0][]
            LB, LS, LS,  // 右: 近,中,远 -> 左大转/左小转/左小转
        },
        {// 中中 [1][1][]
            LS, FW, RS,  // 右: 近,中,远 -> 左小转/直行/右小转
        },
        {// 中远 [1][2][]
            LS, RS, RB,  // 右: 近,中,远 -> 左小转/右小转/右大转
        }
    },
    {// 左远 [2][][]
        {// 中近 [2][0][]
            LS, FW, RS,  // 右: 近,中,远 -> 左小转/直行/右小转
        },
        {// 中中 [2][1][]
            RS, FW, RB,  // 右: 近,中,远 -> 右小转/直行/右大转
        },
        {// 中远 [2][2][]
            RB, RB, FW,  // 右: 近,中,远 -> 右大转/右大转/直行
        }
    }
};

// 规则推理结果
static float sum_numerator, sum_denominator;

/**
 * @brief 计算单个传感器距离值对应的模糊隶属度权重
 * 
 * 该函数将精确的距离数值转换为三个模糊集合（近、中、远）的隶属度，
 * 使用三角形或梯形隶属度函数进行模糊化处理。
 * 
 * @param dist 实际传感器测量的距离值
 * @param weight 指向Fuzzy_Weight结构体的指针，用于存储计算得到的隶属度权重
 */
static void calculateSingleFuzzyWeight(float dist, Fuzzy_Weight *weight) {
    // 1. 计算 近 (Near) 权重
    if (dist <= RANGE_NEAR_START) {
        weight->near = 1.0f;
    } else if (dist >= RANGE_NEAR_END) {
        weight->near = 0.0f;
    } else {
        weight->near = (RANGE_NEAR_END - dist) / (RANGE_NEAR_END - RANGE_NEAR_START);
    }
    // 2. 计算 中 (Medium) 权重 (三角形隶属度)
    if (dist <= RANGE_MEDIUM_START || dist >= RANGE_MEDIUM_END) {
        weight->medium = 0.0f;
    } else if (dist < RANGE_MEDIUM_MEDIUM) {
        weight->medium = (dist - RANGE_MEDIUM_START) / (RANGE_MEDIUM_MEDIUM - RANGE_MEDIUM_START);
    } else {
        weight->medium = (RANGE_MEDIUM_END - dist) / (RANGE_MEDIUM_END - RANGE_MEDIUM_MEDIUM);
    }
    // 3. 计算 远 (Far) 权重
    if (dist <= RANGE_FAR_START) {
        weight->far = 0.0f;
    } else if (dist >= RANGE_FAR_END) {
        weight->far = 1.0f;
    } else {
        weight->far = (dist - RANGE_FAR_START) / (RANGE_FAR_END - RANGE_FAR_START);
    }
}

/**
 * @brief  隶属度函数，计算三个传感器的隶属度权重，数据写入隶属度结构体
 * @param  dist_left, dist_middle, dist_right: 传感器原始距离
 */
static void Fuzzy_weightCalculate(float dist_left, float dist_middle, float dist_right) {
    // 调用辅助函数，分别更新三个全局结构体变量
    calculateSingleFuzzyWeight(dist_left,   &fuzzy_leftWeight);
    calculateSingleFuzzyWeight(dist_middle, &fuzzy_middleWeight);
    calculateSingleFuzzyWeight(dist_right,  &fuzzy_rightWeight);
}

/**
 * @brief 模糊推理函数
 * 
 * 该函数执行模糊推理过程，根据当前的隶属度权重和预定义的规则库，
 * 计算出加权平均的输出值。这是模糊控制系统的核心部分，实现了
 * 从模糊输入到模糊输出的映射。
 * 
 * 推理过程：
 * 1. 遍历所有27条模糊规则（3×3×3）
 * 2. 计算每条规则的激活强度（隶属度的最小值）
 * 3. 根据激活强度对规则输出进行加权求和
 * 4. 通过重心法去模糊化得到最终输出
 */
static void Fuzzy_ruleInference(void) {
    // 每次推理前必须清零，否则角度会无限累加
    sum_numerator = 0.0f;      // 初始化分子累加器（加权输出之和）
    sum_denominator = 0.0f;    // 初始化分母累加器（权重之和）
    
    // 将全局权重存入局部数组，提升索引读取效率
    // 左侧传感器隶属度数组：[近, 中, 远]
    float w_l[3] = {fuzzy_leftWeight.near,   fuzzy_leftWeight.medium,   fuzzy_leftWeight.far};
    // 中央传感器隶属度数组：[近, 中, 远]  
    float w_m[3] = {fuzzy_middleWeight.near, fuzzy_middleWeight.medium, fuzzy_middleWeight.far};
    // 右侧传感器隶属度数组：[近, 中, 远]
    float w_r[3] = {fuzzy_rightWeight.near,  fuzzy_rightWeight.medium,  fuzzy_rightWeight.far};
    
    // 开始 27 条规则的权重计算（三重嵌套循环遍历所有规则）
    for (int i = 0; i < 3; i++) {          // 遍历左侧传感器的3个模糊集
        if (w_l[i] <= 0.0001f) continue;   // 如果该维度权重极小，跳过本次循环（优化性能）
        for (int j = 0; j < 3; j++) {      // 遍历中央传感器的3个模糊集
            if (w_m[j] <= 0.0001f) continue; // 如果该维度权重极小，跳过本次循环
            for (int k = 0; k < 3; k++) {  // 遍历右侧传感器的3个模糊集
                if (w_r[k] <= 0.0001f) continue; // 如果该维度权重极小，跳过本次循环
                
                // 取交集强度：min(L, C, R) - 使用最小值作为合成运算
                // 表示当前输入组合激活规则的程度
                float strength = fminf(w_l[i], fminf(w_m[j], w_r[k]));
                
                // 累加计算：强度 * 规则角度
                // 分子：激活强度乘以对应规则输出的加权和
                sum_numerator += strength * rule_table[i][j][k];    
                // 分母：所有激活强度的总和
                sum_denominator += strength;                        
            }
        }
    }
}

/**
 * @brief 去模糊化函数 - 重心法解模糊
 * 
 * 该函数执行去模糊化过程，将模糊推理得到的模糊输出转换为精确的数值输出。
 * 采用重心法（Center of Gravity）进行解模糊，这是最常用的去模糊化方法之一。
 * 
 * 计算公式：输出 = Σ(激活强度 × 规则输出) / Σ(激活强度)
 * 即加权平均值，其中权重为各规则的激活强度
 * 
 * @return float 解模糊后的精确输出值，表示机器人的转向角度
 *         正值表示右转，负值表示左转，0表示直行
 */
static float Fuzzy_defuzzify(void) {
    // 如果分母太小（即没有任何规则被有效触发），返回 0 (直行) 以保安全
    // 当传感器数据异常或所有规则激活度都很低时，默认直行
    if (sum_denominator < 0.0001f) {
        return 0.0f;
    }
    
    // 计算加权平均后的最终偏置角度
    // 重心法去模糊化核心计算：分子(加权输出总和) / 分母(权重总和)
    // 结果为一个精确的角度值，用于控制机器人的转向
    return (sum_numerator / sum_denominator);
}

/**
 * @brief  模糊控制主接口程序
 * @param  dist_left, dist_middle, dist_right: 传感器原始距离 (cm)
 * @return 最终的避障偏置角度 (Offset)
 */
float Fuzzy_update(float dist_l, float dist_m, float dist_r) {
    // 步骤1：计算各个传感器的隶属度权重
    Fuzzy_weightCalculate(dist_l, dist_m, dist_r);
    // 步骤2：执行规则推理，计算总分子和总分母
    Fuzzy_ruleInference();
    // 步骤3：执行去模糊化并返回最终偏置
    float result = Fuzzy_defuzzify();
    
    return result;
}