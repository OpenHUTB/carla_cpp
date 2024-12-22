import requests  # 导入requests库用于发起网络请求
import argparse  # 导入argparse库用于解析命令行参数
import os  # 导入os库用于操作系统级别的操作
from collections import defaultdict, Counter  # 从collections模块导入defaultdict和Counter用于数据结构操作
import git  # 导入git库用于操作Git库

# 设置命令行参数解析器
# 定义一个命令行工具，允许用户通过命令行传入GitHub的访问令牌
argparser = argparse.ArgumentParser(description='Involvement Degree')
argparser.add_argument('-t', '--token', help='your personal github access token')  # 添加一个命令行参数用于接收GitHub访问令牌
args = argparser.parse_args()  # 解析命令行参数

# 获取并设置GitHub访问令牌和请求头
TOKEN = args.token  # 从命令行参数中获取GitHub访问令牌
headers = {
    'Authorization': f'token {TOKEN}',  # 设置请求头包含授权信息
    'Accept': 'application/vnd.github.v3+json'  # 设置API响应格式为JSON
}

# 设置仓库信息
owner = 'OpenHUTB'  # 仓库所有者
repo = 'carla_cpp'  # 仓库名称

# 定义函数用于统计代码添加和删除行数
def commit_info():
    from git.repo import Repo  # 从git库导入Repo类用于操作Git仓库

    # 初始化本地仓库路径为当前目录
    local_path = os.path.join('.')
    repo = Repo(local_path)  # 创建Repo对象表示本地Git仓库

    # 获取提交日志中的作者名字
    log_info = repo.git.log('--pretty=format:%an')  # 使用git命令获取作者名字
    authors = log_info.splitlines()  # 将日志按行分割成作者列表

    # 定义别名映射，用于标准化作者名字
    alias_map = {
        '王海东': 'donghaiwang',  # 示例别名映射
    }

    # 标准化作者名字，如果作者在别名映射中存在则使用映射后的名字，否则使用原名
    normalized_authors = [alias_map.get(author, author) for author in authors]

    # 使用Counter统计每个作者的提交次数
    author_counts = Counter(normalized_authors)
    print("提交次数：")
    for author, count in author_counts.most_common():  # 按提交次数从高到低打印作者和提交次数
        print(f"{author}: {count} 次提交")

    # 获取提交日志，包含作者名字和增删行数
    log_data = repo.git.log('--pretty=format:%an', '--numstat')  # 使用git命令获取作者名字和增删行数

    # 初始化作者统计字典，每个作者对应一个包含'added'和'deleted'键的字典
    author_stats = defaultdict(lambda: {'added': 0, 'deleted': 0})
    current_author = None  # 初始化当前作者为None

    # 遍历日志的每一行，统计每个作者的增删行数
    for line in log_data.splitlines():
        if '\t' not in line or line.isdigit():  # 如果行中不包括制表符或只包含数字（可能是作者名字）
            current_author = line.strip()  # 更新当前作者为当前行内容（去除首尾空白字符）
        elif '\t' in line:  # 如果行中包含制表符，则解析增删行数
            added, deleted, _ = line.split('\t')  # 将行按制表符分割成增删行数和文件名
            if added != '-':  # 如果增加行数不是'-'（表示没有增加）
                author_stats[current_author]['added'] += int(added)  # 更新当前作者的增加行数
            if deleted != '-':  # 如果删除行数不是'-'（表示没有删除）
                author_stats[current_author]['deleted'] += int(deleted)  # 更新当前作者的删除行数

    # 打印每个作者的增删行数
    for author, stats in author_stats.items():
        print(f"{author}: 添加 {stats['added']} 行, 删除 {stats['deleted']} 行")

# 调用函数统计代码添加和删除行数
commit_info()

# 定义变量用于统计用户提问和评论数
issue_counts = {}  # 用户提问次数字典
comment_counts = {}  # 用户评论次数字典

# 分页获取仓库的issues和comments，统计用户提问和评论数
page = 1  # 初始化页码为1
while True:
    url = f'https://api.github.com/repos/{owner}/{repo}/issues?state=all&per_page=100&page={page}'  # 构造GitHub API请求URL
    response = requests.get(url, headers=headers)  # 发起网络请求
    
    # 处理API响应
    if response.status_code != 200:
        print("请求失败，请检查网络连接或GitHub令牌。")
        break  # 如果请求失败则跳出循环

    # 解析响应数据为JSON格式
    issues = response.json()
    if not issues:
        break  # 如果没有更多issues则跳出循环

    # 遍历所有issues
    for issue in issues:
        if 'pull_request' in issue:  # 如果issue是pull request则跳过
            continue

        # 获取提问用户的登录名，并更新用户提问次数
        user = issue['user']['login']
        issue_counts[user] = issue_counts.get(user, 0) + 1

        # 获取issue的comments URL，并请求comments数据
        comments_url = issue['comments_url']
        comments_response = requests.get(comments_url, headers=headers)
        comments = comments_response.json()

        # 遍历所有comments，更新用户评论次数
        for comment in comments:
            commenter = comment['user']['login']
            comment_counts[commenter] = comment_counts.get(commenter, 0) + 1

    page += 1  # 更新页码以便获取下一页数据

# 将统计结果按照次数从高到低排序
sorted_issue_counts = dict(sorted(issue_counts.items(), key=lambda item: item[1], reverse=True))
sorted_comment_counts = dict(sorted(comment_counts.items(), key=lambda item: item[1], reverse=True))

# 打印统计结果
print("提问次数：")
for user, count in sorted_issue_counts.items():
    print(f"{user}: {count}")  # 按提问次数从高到低打印用户和提问次数

print("\n回答次数：")
for user, count in sorted_comment_counts.items():
    print(f"{user}: {count}")  # 按评论次数从高到低打印用户和评论次数counts.items():
    print(f"{user}: {count}")
