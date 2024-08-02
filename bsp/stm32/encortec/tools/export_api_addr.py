import re
import sys

def read_api_list(api_list_path):
    """读取api_list.txt文件，返回函数名称列表"""
    with open(api_list_path, 'r') as api_file:
        return [line.strip() for line in api_file]

def parse_map_file(map_file_path, api_list, output_h_file_path):
    # 定义正则表达式来匹配函数名和地址，排除以".text"开头的行
    pattern = re.compile(
        r'^(?!\.text)\s+0x(?P<address>[0-9a-fA-F]+)\s+(?P<func_name>\w+)',
        re.MULTILINE
    )

    found_apis = set()

    # 准备输出文件
    with open(output_h_file_path, 'w') as output_file:
        # 输出头文件的预处理器指令
        output_file.write("#ifndef __FUNCTION_ADDRESSES_H\n")
        output_file.write("#define __FUNCTION_ADDRESSES_H\n\n")

        # 逐行读取map文件
        with open(map_file_path, 'r') as map_file:
            for line in map_file:
                match = pattern.search(line)
                if match:
                    func_name = match.group('func_name')
                    address = int(match.group('address'), 16)
                    if address != 0 and func_name in api_list:
                        output_file.write(f"#define {func_name}_addr 0x{(address+1):08X}\n")
                        found_apis.add(func_name)

        # 结束头文件的预处理器指令
        output_file.write("\n#endif /* __FUNCTION_ADDRESSES_H */")

    # 检查是否有API未在.map文件中找到
    missing_apis = set(api_list) - found_apis
    if missing_apis:
        print("Warning: These APIs are missing from the map file:", file=sys.stderr)
        for api in missing_apis:
            print(f"    '{api}'", file=sys.stderr)

def export_api_addr(map_file_path, api_list_path, output_h_file_path):
    api_list = read_api_list(api_list_path)
    parse_map_file(map_file_path, api_list, output_h_file_path)