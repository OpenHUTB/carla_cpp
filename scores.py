import requests

import argparse

from collections import defaultdict
from collections import Counter

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

commit_info()


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

sorted_issue_counts = dict(sorted(issue_counts.items(), key=lambda item: item[1], reverse=True))
sorted_comment_counts = dict(sorted(comment_counts.items(), key=lambda item: item[1], reverse=True))

# 输出结果
print("提问次数：")
for user, count in sorted_issue_counts.items():
    print(f"{user}: {count}")

print("\n回答次数：")
for user, count in sorted_comment_counts.items():
    print(f"{user}: {count}")