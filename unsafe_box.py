broken_data = {
  "id": 102,
  "name": "NewUser"
  # preferences 字段丢失了！
}

def get_theme(user_data):
    # TODO: 请在这里写代码
    prefs = user_data.get("preference",{})
    
    ui_settings = prefs.get("ui", {})

    return ui_settings.get("theme", "default_light")
    # 目标：安全地获取 user_data['preferences']['ui']['theme']
    # 如果拿不到，返回 'default_light'
    pass

# 测试
print(f"用户主题是: {get_theme(broken_data)}")
