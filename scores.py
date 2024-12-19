import requests#导入request模块，用于请求

import argparse#导入argparse模块，用于处理命令行参数
import os#导入os模块，用于提供与操作系统交互

from collections import defaultdict
from collections import Counter

import git  # 导入git模块，用于操作Git库

argparser = argparse.ArgumentParser(
        description='Involvement Degree')
argparser.add_argument(
    '-t', '--token',
    help='your personal github access token')
args = argparser.parse_args()

# 替换为你的个人访问令牌（可选，但推荐）
TOKEN = args.token

headers = {
    'Authorization': f'token {TOKEN}',
    'Accept': 'application/vnd.github.v3+json'
}

owner = 'OpenHUTB'  # 替换为仓库所有者
repo = 'carla_cpp'    # 替换为仓库名称


#########################################
####### 统计代码添加和删除行数 ############
#########################################
def commit_info():
    import os
    from git.repo import Repo

    local_path = os.path.join('.')
    repo = Repo(local_path)


    # 获取提交日志，格式为作者名字
    log_info = repo.git.log('--pretty=format:%an')

    # 将提交日志按行分割
    authors = log_info.splitlines()

    # 定义别名映射
    alias_map = {
        '王海东': 'donghaiwang',
        # 你可以在这里添加更多的别名
    }

    # 将所有作者名字替换为标准化名字
    normalized_authors = [alias_map.get(author, author) for author in authors]

    # 使用 Counter 统计每个标准化作者的提交次数
    author_counts = Counter(normalized_authors)

    # 打印统计结果
    print("提交次数：")
    for author, count in author_counts.most_common():
        print(f"{author}: {count} 次提交")

    # 获取提交日志，格式为作者名字，并包含增删行数
    log_data = repo.git.log('--pretty=format:%an', '--numstat')

    # 处理 log 数据
    author_stats = defaultdict(lambda: {'added': 0, 'deleted': 0})  # 使用 defaultdict 来统计每个作者的增加行数
    current_author = None

    line_cnt = 0
    # 解析日志，统计每个作者的增加行数
    for line in log_data.splitlines():
        line_cnt = line_cnt + 1
        if line.strip() == "":
            continue
        # 如果是作者行，则更新当前提交的作者（作者有可能是数字开头）
        if '\t' not in line or line.isdigit():  # 提交者名字的行不会以数字开头
            current_author = line.strip()
        elif '\t' in line:
            # 解析 numstat 格式的增删行
            added, deleted, _ = line.split('\t')
            if added != '-':  # 处理新增的行数
                author_stats[current_author]['added'] += int(added)
            if deleted != '-':  # 处理删除的行数
                author_stats[current_author]['deleted'] += int(deleted)

    # 输出每个作者的增加行数
    for author, stats in author_stats.items():
        print(f"{author}: 添加 {stats['added']} 行, 删除 {stats['deleted']} 行")
    pass

commit_info()



#########################################
####### 统计用户提问和评论数 ##############
#########################################
# 初始化统计字典
issue_counts = {}
comment_counts = {}

page = 1
while True:
    url = f'https://api.github.com/repos/{owner}/{repo}/issues?state=all&per_page=100&page={page}'
    # 需要把代理关掉，否则报错（原因不明）：urllib3.exceptions.MaxRetryError: HTTPSConnectionPool
    response = requests.get(url, headers=headers)
    issues = response.json()

    if not issues:
        break

    for issue in issues:
        # 过滤掉 Pull Requests
        if 'pull_request' in issue:
            continue

        # 统计提问者
        user = issue['user']['login']
        issue_counts[user] = issue_counts.get(user, 0) + 1

        # 获取 Issue 的评论
        comments_url = issue['comments_url']
        comments_response = requests.get(comments_url, headers=headers)
        comments = comments_response.json()

        for comment in comments:
            commenter = comment['user']['login']
            comment_counts[commenter] = comment_counts.get(commenter, 0) + 1

    page += 1

#按数量排序issue_counts和comment_counts
sorted_issue_counts = dict(sorted(issue_counts.items(), key=lambda item: item[1], reverse=True))
sorted_comment_counts = dict(sorted(comment_counts.items(), key=lambda item: item[1], reverse=True))

# 输出结果
print("提问次数：")
for user, count in sorted_issue_counts.items():
    print(f"{user}: {count}")

print("\n回答次数：")
for user, count in sorted_comment_counts.items():
    print(f"{user}: {count}")
