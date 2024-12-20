import requests  # 用于发起网络请求
import argparse  # 用于解析命令行参数
import os  # 用于操作系统级别的操作
from collections import defaultdict, Counter  # 用于数据结构操作
import git  # 用于操作Git库

# 设置命令行参数解析
argparser = argparse.ArgumentParser(description='Involvement Degree')
argparser.add_argument('-t', '--token', help='your personal github access token')
args = argparser.parse_args()

# 获取GitHub访问令牌
TOKEN = args.token

# 设置请求头
headers = {
    'Authorization': f'token {TOKEN}',
    'Accept': 'application/vnd.github.v3+json'
}

# 仓库信息
owner = 'OpenHUTB'  # 仓库所有者
repo = 'carla_cpp'  # 仓库名称

#########################################
####### 统计代码添加和删除行数 ########
#########################################
def commit_info():
    from git.repo import Repo

    # 初始化本地仓库路径
    local_path = os.path.join('.')
    repo = Repo(local_path)

    # 获取提交日志，格式为作者名字
    log_info = repo.git.log('--pretty=format:%an')
    authors = log_info.splitlines()

    # 定义别名映射
    alias_map = {
        '王海东': 'donghaiwang',
    }

    # 标准化作者名字
    normalized_authors = [alias_map.get(author, author) for author in authors]

    # 统计每个作者的提交次数
    author_counts = Counter(normalized_authors)
    print("提交次数：")
    for author, count in author_counts.most_common():
        print(f"{author}: {count} 次提交")

    # 获取提交日志，格式为作者名字，并包含增删行数
    log_data = repo.git.log('--pretty=format:%an', '--numstat')

    # 统计每个作者的增加行数
    author_stats = defaultdict(lambda: {'added': 0, 'deleted': 0})
    current_author = None
    for line in log_data.splitlines():
        if '\t' not in line or line.isdigit():
            current_author = line.strip()
        elif '\t' in line:
            added, deleted, _ = line.split('\t')
            if added != '-':
                author_stats[current_author]['added'] += int(added)
            if deleted != '-':
                author_stats[current_author]['deleted'] += int(deleted)

    # 输出每个作者的增加行数
    for author, stats in author_stats.items():
        print(f"{author}: 添加 {stats['added']} 行, 删除 {stats['deleted']} 行")

commit_info()

#########################################
####### 统计用户提问和评论数 ##############
#########################################
issue_counts = {}
comment_counts = {}

page = 1
while True:
    url = f'https://api.github.com/repos/{owner}/{repo}/issues?state=all&per_page=100&page={page}'
    response = requests.get(url, headers=headers)
    
    if response.status_code != 200:
        print("请求失败，请检查网络连接或GitHub令牌。")
        break
    #如果 回应状态密码不等于200
      打印("请求失败，请检查网络连接或GitHub令牌。")
      结束循环

    issues = response.json()#从GitHub API获取的响应对象转换为JSON格式，并赋值给变量 issues
    if not issues:
        break
    #如果 issues 为空，即没有更多的问题可以处理，那么使用 break 语句跳出循环。

    for issue in issues: #遍历 issues 列表中的每个问题
        if 'pull_request' in issue:
            continue
        #如果当前的问题实际上是一个拉取请求（pull request），则跳过当前循环的剩余部分，继续处理下一个问题

        user = issue['user']['login'] #获取创建当前问题的用户的登录名
        issue_counts[user] = issue_counts.get(user, 0) + 1 #在字典 issue_counts 中为该用户增加问题计数。如果用户之前没有记录，则从0开始计数

        comments_url = issue['comments_url']
        comments_response = requests.get(comments_url, headers=headers)
        comments = comments_response.json()

        for comment in comments:
            commenter = comment['user']['login']
            comment_counts[commenter] = comment_counts.get(commenter, 0) + 1

    page += 1

sorted_issue_counts = dict(sorted(issue_counts.items(), key=lambda item: item[1], reverse=True))
sorted_comment_counts = dict(sorted(comment_counts.items(), key=lambda item: item[1], reverse=True))

print("提问次数：")
for user, count in sorted_issue_counts.items():
    print(f"{user}: {count}")

print("\n回答次数：")
for user, count in sorted_comment_counts.items():
    print(f"{user}: {count}")
