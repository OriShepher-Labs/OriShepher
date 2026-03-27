# OriShepher-Core仓库说明

使用HAL库开发。

名字是乱起的……Origami Shepherd。

### [分支说明](https://github.com/OriShepher-Labs/OriShepher-Core/branches)

1. [`main`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/main) 分支是<u>大版本</u>分支。

   当积累足够多的更新，在适当时机，经过多次测试确保没有明显 bug，并进行代码规范化整理后可由 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop) 分支合并到该分支。
2. [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop) 分支是<u>主要开发</u>分支。

   组员在各自的测试分支 `feature-xxx` 中确认自己所开发功能的<u>小版本阶段性目标实现</u>，经过测试并检查无明显 bug 后的更新，进行代码规范化整理后可合并到该分支。
3. `feature-xxx` 分支是各组员的功能开发分支。

   组员自行创建feature分支，并在各自的分支进行<u>开发和调试</u>。命名方式为 `feature-(功能名)`，要使用英文，如 `featrue-CPG`、`feature-oled`、`feature-remote`。若有多个组员同时开发一个功能，可以共同使用同一个分支(不建议)，或更名，或增加后缀 `feature-remote-B`来区别。使用多分支时，需先合并到其中一个分支，解决所有代码冲突后，再提交拉取请求合并到 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop)。

### [拉取请求](https://github.com/OriShepher-Labs/OriShepher-Core/pulls)规范

1. 由 `feature-xxx` 发起的拉取请求需有除自己外的其他至少 1 名组员审查后，才允许合并到 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop)。
2. 大版本更新时，轮流指定一人，由 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop) 提交拉取请求到 [`main`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/main)，需要所有人的审查后，才允许合并到 [`main`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/main)。
3. 拉取请求通过后，如此分支暂无后续开发计划可以关闭/删除此`feature-xxx`分支。

### 开发规范

开发时请尽量按照我们建议的命名方式和代码格式进行开发，包括函数、变量名、提交信息等等。相关规范参考文件在项目的`docs\命名规范.md`中。

当然这个规范其实也是不够规范的（专业）。

请不要写屎山代码💩。

### 其他说明

不知道什么是分支？不知道什么是拉取请求？不会用 Github？可以到哔哩哔哩或询问 AI 学习。

<span data-type="text" style="color:#bbbebf;">这个仓库是私有的，不开源，只是方便管理和同步代码，请不要未经所有核心贡献者同意的情况下往开源仓库上传项目核心代码。</span>

[`OriShepher`](https://github.com/OriShepher-Labs/OriShepher)<span data-type="text" style="color:#bbbebf;"> 仓库才是用于开源和展示(占坑、证明)的仓库，预计未来会上传项目的自定义库程序头文件等非核心文件用于展示本项目。其他私有仓库只是用于开发和测试。以此方法来保证我们的成果得到展示和证明且不易被窃取。</span>

项目使用[Apache-2.0 许可证](https://github.com/OriShepher-Labs/OriShepher-Core?tab=Apache-2.0-1-ov-file#)。

GitHub仍有许多有趣的功能等待我们去发现……

---

<span data-type="text" style="color: var(--b3-font-color5);">所有的说明和规范都尚未完善，它们应在我们的共同讨论中被持续改进……</span>