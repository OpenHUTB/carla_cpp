import requests  # 用于发起网络请求
import argparse  # 用于解析命令行参数
import os  # 用于操作系统级别的操作
from collections import defaultdict, Counter  # 用于数据结构操作
import git  # 用于操作Git库

# 设置命令行参数解析
#通过argparse库，代码定义了一个命令行工具，允许用户通过命令行传入GitHub的访问令牌
argparser = argparse.ArgumentParser(description='Involvement Degree')
argparser.add_argument('-t', '--token', help='your personal github access token')
args = argparser.parse_args()

# 获取GitHub访问令牌
TOKEN = args.token
#设置请求头包含授权信息
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
    #初始化当前作者为None
    current_author = None
    #遍历日志的每一行
    for line in log_data.splitlines():
        #如果行中不包括制表符（\t）或者行内容为空
        if '\t' not in line or line.isdigit():
            #将当前作者设为该行内容（去除首尾空白字符）
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
#处理API响应    
    if response.status_code != 200:
        print("请求失败，请检查网络连接或GitHub令牌。")
        break

    #将响应数据解析为JSON格式
    issues = response.json()
    if not issues:
        #跳出循环
        break

    #遍历所有问题
    for issue in issues:
        if 'pull_request' in issue:
            #跳过当前循环，继续下一个问题的处理
            continue

        #获取问题的用户登陆名
        user = issue['user']['login']
        #更新用户提出问题的计数
        issue_counts[user] = issue_counts.get(user, 0) + 1

        comments_url = issue['comments_url']#是从一个名为issue的字典（假设issue是一个字典类型的数据结构）中获取键为comments_url的值，并将这个值赋给变量comments_url。这样做的目的是为了得到与某个问题（issue）相关的评论的URL地址，以便后续获取评论数据。
        comments_response = requests.get(comments_url, headers=headers)#这里使用requests库（假设已经正确导入）的get方法来发送一个HTTP GET请求到comments_url所指定的地址。headers是请求头信息，可能包含一些认证信息、用户代理等内容。这个操作的目的是获取包含评论信息的响应内容。
        comments = comments_response.json()#由于从服务器获取的响应内容通常是JSON格式的数据（这是一种常见的用于数据交换的格式），所以使用json方法将响应内容解析为Python中的数据结构（如字典、列表等），并将解析后的结果赋给comments变量，这样就可以方便地在Python中操作这些评论数据了。

        for comment in comments:
            commenter = comment['user']['login']
            comment_counts[commenter] = comment_counts.get(commenter, 0) + 1#这里假设comment_counts是一个字典。comment_counts.get(commenter, 0)这部分是尝试从comment_counts字典中获取键为commenter的值，如果键不存在，则返回默认值0。然后将这个值加1，并重新将结果赋给comment_counts字典中键为commenter的项。这个操作的目的是统计每个用户发表评论的数量。


    page += 1

sorted_issue_counts = dict(sorted(issue_counts.items(), key=lambda item: item[1], reverse=True))
sorted_comment_counts = dict(sorted(comment_counts.items(), key=lambda item: item[1], reverse=True))
#将统计结果按照次数从高到低排序并打印出来
print("提问次数：")
for user, count in sorted_issue_counts.items():
    print(f"{user}: {count}")

print("\n回答次数：")
for user, count in sorted_comment_counts.items():
    print(f"{user}: {count}")
