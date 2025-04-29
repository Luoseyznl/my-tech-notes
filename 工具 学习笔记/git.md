### 分布式版本控制系统 Git

- Git 允许每个开发者拥有完整的本地仓库副本，包括全部历史记录，支持离线操作（如提交、查看历史），而 CVS 和 SVN 依赖中央仓库，网络断开时功能受限。

#### 1. 用法

1. 创建版本库 `$ git init`

2. 添加到仓库 `$ git add readme.txt`
3. 提交到仓库（快照） `$ git commit -m "wrote a readme file"`
4. 版本控制：
   1. 仓库当前的状态 `$ git status` `$ git diff readme.txt `
   2. 查看提交日志 `$ git log --pretty=oneline` （版本号 commit id 由 SHA1 计算（HEAD 表示当前版本号））
   3. 版本回退 `$ git reset --hard HEAD^` （回退到上个版本的已提交状态（通过 HEAD 指针移动来控制版本））
   4. 查看命令历史 `git reflog`

#### 2. 工作区和暂存区

- `.git` 版本库：git add 把文件修改添加到暂存区；git commit 把暂存区的所有内容提交到当前分支（初始 master）。
- 撤销修改（回到添加到暂存区后的状态） `$ git checkout -- readme.txt`
- 删除文件 `git rm` 删掉，并且 `git commit`

#### 3. 远程仓库 GitHub

1. 支持 SSH 协议
2. 克隆（使用 ssh 协议） `$ git clone git@github.com:XXX/XXX.git`

#### 4. 分支管理

1. 查看分支 `$ git branch` 
2. 创建并切换分支 `$ git checkout -b dev` `$ git switch -c dev` 切换分支 `git checkout <name>` `git switch <name>`
3. 把 dev 分支的工作成果合并到 master 分支上 `$ git checkout master` `$ git merge dev`
4. 删除分支 `$ git branch -d dev`
5. 禁用 Fast forward 模式合并（生成 commit）`$ git merge --no-ff -m "merge with no-ff" dev`

#### 5. 打标签
`$ git tag v1.0` `$ git tag -d v0.1`
`$ git log --pretty=oneline --abbrev-commit` `$ git tag v0.9 f52c633`
