import requests

import argparse

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
sorted_comment_counts = dict(sorted(comment_counts.items(), key=lambda item: item[1], reversed=True))

# 输出结果
print("提问次数：")
for user, count in sorted_issue_counts.items():
    print(f"{user}: {count}")

print("\n回答次数：")
for user, count in sorted_comment_counts.items():
    print(f"{user}: {count}")