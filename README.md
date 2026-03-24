# OriShepher-Core仓库说明

名字是乱起的……

### 分支说明

1. `main` 分支是<u>大版本</u>分支，当积累足够多的更新，并确保没有明显 bug，经过整理后可由 `develop` 分支合并到该分支。
2. `develop` 分支是主要开发分支，各组员的测试分支 `feature-xxx` 确认自己所开发功能的<u>小版本目标实现</u>，并检查无明显 bug 后的更新，可合并到该分支。
3. `feature-xxx` 分支是各组员的开发分支。组员自行创建分支，并在各自的分支进行<u>开发和调试</u>。命名方式为 `feature-(功能名)`，要使用英文，如 `featrue-CPG`、`feature-oled`、`feature-remote`。若有多个组员同时开发一个功能，可以共同使用同一个分支，或更名，或增加后缀 `feature-remote-B`，使用多分支时，需先合并再提交拉取请求合并到 `develop`。

### 拉取请求规范

1. 由 `feature-xxx` 发起的拉取请求需有除自己外的其他至少 1 名组员审查后，才允许合并到 `develop`。
2. 大版本更新时，轮流指定一人，由 `develop` 提交拉取请求到 `main`，需要所有人的审查后，才允许合并到 `main`。

### 其他说明

不知道什么是分支？不知道什么是拉取请求？不会用 Github？可以到哔哩哔哩或询问 AI 学习。

这个仓库是私有的，不开源，只是方便管理和同步代码，请不要往开源仓库传核心代码。

---

<span data-type="text" style="color: var(--b3-font-color5); --darkreader-inline-color: var(--darkreader-text--b3-font-color5, var(--darkreader-text-000000, #ffffff));">持续改进中……</span>