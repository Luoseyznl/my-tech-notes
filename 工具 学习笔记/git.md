# Git 实用指南

## 1. 常用配置

```bash
# 设置用户信息
git config --global user.name "Your Name"     # 设置全局用户名
git config --global user.email "you@example.com"  # 设置全局邮箱

# 配置编辑器
git config --global core.editor "vim"  # 设置Git默认编辑器

# 查看配置
git config --list  # 查看所有配置
```

## 2. 分支管理

```bash
# 查看分支
git branch            # 列出本地分支，当前分支前有星号(*)
git branch -r         # 列出远程跟踪分支(remote branches)
git branch -a         # 列出所有分支(本地和远程)
git branch -v         # 查看分支最后一次提交

# 创建与切换分支
git branch <branch_name>           # 创建新分支(不切换)
git checkout <branch_name>         # 切换到已有分支(旧方式)
git switch <branch_name>           # 切换到已有分支(新方式，推荐)
git checkout -b <branch_name>      # 创建并切换到新分支(旧方式)
git switch -c <branch_name>        # 创建并切换到新分支(新方式，推荐)

# 删除分支
git branch -d <branch_name>        # 删除本地分支(安全，合并后才能删除)
git branch -D <branch_name>        # 强制删除本地分支(即使未合并)
git push origin --delete <branch_name>  # 删除远程分支
```

## 3. 分支同步与合并

```bash
# 远程分支同步
git fetch origin                  # 获取远程所有分支的最新状态
git fetch origin <branch_name>    # 获取特定远程分支的最新状态

# 本地分支同步到远程状态
git checkout <branch_name>        # 切换到要同步的分支
git reset --hard origin/<branch_name>   # 完全同步到远程状态(丢弃所有本地修改！)

# 合并更新
git pull origin <branch_name>      # 拉取并合并(fetch + merge)
git pull --rebase origin <branch_name>  # 拉取并变基(fetch + rebase)，保持提交历史整洁
```

## 4. 提交与回滚

```bash
# 提交修改
git add <file>             # 添加特定文件到暂存区
git add .                  # 添加所有修改到暂存区
git commit -m "消息"        # 提交暂存区内容
git commit -am "消息"       # 合并add和commit操作(仅对已跟踪文件)
git commit --amend         # 修改最近一次提交(包括提交消息和内容)

# 查看历史
git log                              # 完整提交历史
git log --oneline                    # 简洁单行显示提交历史
git log --oneline --graph --decorate --all  # 图形化显示所有分支提交历史

# 回退操作
git reset --soft HEAD~1    # 撤销最近一次提交，保留修改在暂存区
git reset --mixed HEAD~1   # 撤销最近一次提交，保留修改在工作区(默认)
git reset --hard HEAD~1    # 撤销最近一次提交，丢弃所有修改(危险！)
git revert <commit_id>     # 创建新提交来撤销指定提交的修改(安全)
```

## 5. 合并与冲突管理

```bash
# 合并分支
git merge <branch_name>             # 将指定分支合并到当前分支
git merge --no-ff <branch_name>     # 创建合并提交，即使可以快进
git merge --abort                   # 中止合并操作(解决冲突前)

# 解决冲突
# 1. 编辑冲突文件，查找并解决 <<<<<<< HEAD ... ======= ... >>>>>>> 标记
# 2. 标记为已解决并继续
git add <conflicted_file>           # 标记文件冲突已解决
git merge --continue                # 继续合并过程
# 或
git commit                          # 手动提交解决后的合并
```

## 6. Rebase 操作

```bash
# 基本变基
git rebase <branch_name>        # 将当前分支变基到指定分支

# 交互式变基
git rebase -i HEAD~3            # 交互式变基最近3个提交
# 常用选项:
#   pick    - 保留提交
#   reword  - 修改提交信息
#   edit    - 修改提交内容
#   squash  - 合并到前一个提交
#   fixup   - 合并到前一个提交(丢弃提交信息)
#   drop    - 删除提交

# 变基流程控制
git rebase --abort              # 放弃变基操作
git rebase --continue           # 解决冲突后继续变基
git rebase --skip               # 跳过当前补丁
```

## 7. 清理与恢复

```bash
# 撤销工作区修改
git checkout -- <file>          # (旧方式)撤销指定文件的修改
git restore <file>              # (新方式)撤销指定文件的修改
git restore .                   # 撤销所有工作区更改

# 撤销暂存区修改
git reset HEAD <file>           # (旧方式)将文件从暂存区移回工作区
git restore --staged <file>     # (新方式)将文件从暂存区移回工作区
git restore --staged .          # 撤销所有已暂存更改

# 清理未跟踪文件
git clean -n                    # 预览将被删除的未跟踪文件
git clean -f                    # 删除未跟踪文件
git clean -fd                   # 删除未跟踪文件和目录
git clean -fx                   # 删除未跟踪和.gitignore中的文件
```

## 8. 其他技巧

```bash
# 临时存储
git stash                       # 保存未提交更改
git stash save "描述信息"        # 带描述保存未提交更改
git stash list                  # 查看所有存储记录
git stash pop                   # 恢复最近的存储并删除记录
git stash apply stash@{n}       # 应用特定存储但不删除
git stash drop stash@{n}        # 删除特定存储

# 文件历史与比较
git log -- <file_path>          # 查看特定文件的修改历史
git blame <file_path>           # 查看文件每一行的最后修改者
git diff <file_path>            # 查看工作区文件与暂存区的差异
git diff --staged <file_path>   # 查看暂存区文件与上次提交的差异
git diff HEAD~1 <file_path>     # 比较文件与前一次提交的差异

# 远程操作
git remote -v                   # 查看远程仓库
git remote add <name> <url>     # 添加远程仓库
git push -u origin <branch>     # 推送并设置上游分支
git push --force-with-lease     # 安全的强制推送(检查远程更新)
```

## 9. 常见工作流程

```bash
# 功能分支工作流
git switch -c feature/new-feature   # 创建功能分支
# 开发并提交...
git push -u origin feature/new-feature  # 推送到远程
# 在GitHub/GitLab创建Pull Request或Merge Request
# 审核后合并

# 紧急修复流程
git switch -c hotfix/issue-123 main     # 从主分支创建修复分支
# 修复问题并提交...
git push -u origin hotfix/issue-123     # 推送修复分支
# 合并到主分支后同步到开发分支
git switch develop
git merge main                          # 将修复同步到开发分支
```

