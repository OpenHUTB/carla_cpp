import requests
import argparse
import os
from collections import defaultdict, Counter
import git

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
def count_contributions(url, counts_dict):
    try:
        response = requests.get(url, headers=headers)
        response.raise_for_status()  # 将触发异常，如果状态码不是 200
        issues = response.json()
        for issue in issues:
            if 'pull_request' in issue:
                continue
            user = issue['user']['login']
            counts_dict[user] = counts_dict.get(user, 0) + 1
            comments_url = issue['comments_url']
            comments_response = requests.get(comments_url, headers=headers)
            comments_response.raise_for_status()
            comments = comments_response.json()
            for comment in comments:
                commenter = comment['user']['login']
                counts_dict[commenter] = counts_dict.get(commenter, 0) + 1
    except requests.exceptions.RequestException as e:
        print(f"请求失败：{e}")

issue_counts = {}
comment_counts = {}

page = 1
while True:
    url = f'https://api.github.com/repos/{owner}/{repo}/issues?state=all&per_page=100&page={page}'
    count_contributions(url, issue_counts)
    if not issue_counts:
        break
    page += 1

sorted_issue_counts = dict(sorted(issue_counts.items(), key=lambda item: item[1], reverse=True))
sorted_comment_counts = dict(sorted(comment_counts.items(), key=lambda item: item[1], reverse=True))

print("提问次数：")
for user, count in sorted_issue_counts.items():
    print(f"{user}: {count}")

print("\n回答次数：")
for user, count in sorted_comment_counts.items():
    print(f"{user}: {count}")
