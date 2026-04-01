使用HAL库开发。

### [分支(Branches)](https://github.com/OriShepher-Labs/OriShepher-Core/branches)说明

1. [`main`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/main) 分支是**大版本**分支。

   当积累足够多的更新，在适当时机，经过多次测试确保没有明显 bug，并进行代码规范化整理后，可由 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop) 分支合并到该分支。
2. [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop) 分支是**主要开发**分支。

   成员在各自的测试分支 `feature-xxx` 中所开发的功能、优化、修改等，经过测试并检查无明显 bug ，进行代码规范化整理后，可合并到该分支。

   必要或关键的修改，建议尽快提交拉取请求到 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop)，如底层驱动的更新或修改函数接口等。这类所有人都要用到的关键文件，需要尽快同步修改到所有人。因此各成员也需要留意 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop) 分支是否有代码更新，并及时拉取新代码。
3. `feature/fix-xxx` 分支是各成员的功能开发分支。

   成员自行创建feature分支，并在各自的分支进行**开发和调试**。命名方式为 `feature/fix-(功能/内容)`，要使用英文，如 `featrue-CPG`、`feature-oled`、`feature-remote`、`fix-SBUS`、`fix-CPG-function`。

   若有多个成员同时开发一个功能，可以共同使用同一个分支(不建议)，或更名，或增加后缀 `feature-remote-B`来区别。使用多分支时，提交拉取请求前，需先合并到其中一个分支，拉取 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop) 的最新代码，解决所有代码冲突后，再提交拉取请求合并到 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop)。

   若修改了底层驱动的更新或修改函数接口等，请尽快提交拉取请求。

### [拉取请求](https://github.com/OriShepher-Labs/OriShepher-Core/pulls)(Pull Request)规范

1. 提交拉取请求前先拉取 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop) 的最新代码，解决所有代码冲突后，再提交拉取请求合并到 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop)。
2. 可以在开发完成前先提交**拉取请求草案(Pull request as draft)** ，来得到成员的检查和讨论，期间仍可正常进行提交和修改，参与讨论的成员会在请求提交者提交新的commit或有新的讨论消息时收到通知，当一切就绪后再转为准备审核(Ready for review)。
3. 由 `feature-xxx` 发起的拉取请求需有除自己外的其他**至少 1 名成员(除自己)审查**后，才允许合并到 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop)。
4. 大版本更新时，轮流指定一人，由 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop) 提交拉取请求到 [`main`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/main)，需要**所有成员的审查**后，才允许合并到 [`main`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/main)。
5. 由于仓库是私有仓库且没氪金，分支保护和拉取请求规则无法生效，规范全靠大家自觉遵守。
6. 拉取请求通过后，如该分支暂无后续开发计划可以选择关闭/删除此 `feature-xxx` 分支。

### 开发规范

开发时请尽量按照我们建议的命名方式和代码格式进行开发，包括函数、变量名、提交信息等等。相关规范参考文件在项目的[`docs\STYLE_RULES.md`](STYLE_RULES.md)中。

可以在提交(commit)和拉取请求(pull request)的提交信息中使用中文，看得懂就行。

当然这个规范其实也是不够规范的（专业）。

拜托请不要写屎山代码💩。

### 其他说明

不知道什么是分支？不知道什么是拉取请求？不会用 Github？可以到哔哩哔哩或询问 AI 学习。

项目使用[Apache-2.0 许可证](https://github.com/OriShepher-Labs/OriShepher-Core?tab=Apache-2.0-1-ov-file#)。

GitHub仍有许多有趣的功能等待我们去发现……

---

<span data-type="text" style="color: var(--b3-font-color5); --darkreader-inline-color: var(--darkreader-text--b3-font-color5, var(--darkreader-text-000000, #ffffff));">所有的说明和规范都尚未完善，它们应在我们的共同讨论中被持续改进……</span>