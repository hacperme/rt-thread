import re
import sys

def read_api_list(api_list_path):
    """读取api_list.txt文件，返回函数名称列表"""
    with open(api_list_path, 'r') as api_file:
        return [line.strip() for line in api_file]

def parse_map_file(map_file_path, api_list, output_h_file_path):
    # 正则表达式匹配函数名称和地址
    pattern = r'\.text\.(?P<func_name>\w+)\s+0x(?P<address>[0-9a-fA-F]+)'

    # 打开map文件并读取内容
    with open(map_file_path, 'r') as map_file:
        content = map_file.read()

    # 使用正则表达式查找所有匹配项
    matches = re.findall(pattern, content)

    found_apis = set()  # 用于跟踪在.map文件中找到的API

    # 准备输出文件
    with open(output_h_file_path, 'w') as output_file:
        # 输出头文件的预处理器指令
        output_file.write("#ifndef __ENCORTEC_FUNCTION_ADDRESSES_H\n")
        output_file.write("#define __ENCORTEC_FUNCTION_ADDRESSES_H\n\n")

        # 输出每个函数的地址定义，但只输出api_list中的函数
        for match in matches:
            func_name = match[0]
            address = int(match[1], 16)  # 将十六进制地址转换为整数
            if address != 0 and func_name in api_list:  # 检查地址是否非零且函数在api_list中
                output_file.write(f"#define {func_name}_addr 0x{(address+1):08X}\n")  # 格式化输出地址
                found_apis.add(func_name)  # 标记此API已找到

        # 结束头文件的预处理器指令
        output_file.write("\n#endif /* __ENCORTEC_FUNCTION_ADDRESSES_H */")

    # 检查是否有API未在.map文件中找到
    missing_apis = set(api_list) - found_apis
    # if missing_apis:
    #     raise ValueError(f"The following APIs are missing from the map file: {', '.join(missing_apis)}")
    if missing_apis:
        print("Warning: These APIs 'are missing from the map file:", file=sys.stderr)
        for api in missing_apis:
            print(f"    '{api}'", file=sys.stderr)

def export_api_addr(map_file_path, api_list_path, output_h_file_path):
    api_list = read_api_list(api_list_path)
    parse_map_file(map_file_path, api_list, output_h_file_path)
