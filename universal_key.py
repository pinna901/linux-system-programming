def safe_get(data, path, default=None):
    current_layer = data

    for key in path:
        # 第一关：现在的脚下必须是字典，才能继续往下走
        # 如果 current_layer 变成了字符串(例如 "dark")或列表，就没法再根据 key 找了
        if not isinstance(current_layer, dict):
            return default

        # 第二关：字典里得有这个 key
        if key not in current_layer:
            return default

        # 通关：进入下一层 (就像特修斯之船，current_layer 的身份变了)
        current_layer = current_layer[key]

    # 循环走完，说明路径全通，手里拿的就是最终结果
    return current_layer

# --- 测试数据 ---
user_profile = {
    "name": "Pinna",
    "settings": {
        "visual": {
            "mode": "dark"
        }
    }
}

# 测试 1: 正常获取
print(f"模式: {safe_get(user_profile, ['settings', 'visual', 'mode'])}") 
# 预期: dark

# 测试 2: 路径断裂
print(f"音量: {safe_get(user_profile, ['settings', 'audio', 'vol'], 0)}")
# 预期: 0

# 测试 3: 半路撞墙 (试图在字符串 'dark' 里找 sub_setting)
print(f"子设置: {safe_get(user_profile, ['settings', 'visual', 'mode', 'sub'], 'Not Found')}")
# 预期: Not Found
