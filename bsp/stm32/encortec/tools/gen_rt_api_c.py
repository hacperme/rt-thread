import re

def gen_rt_api_c(rt_api_typedef_h_file, rt_api_c_file):

    # 读取头文件内容
    with open(rt_api_typedef_h_file, 'r') as file:
        content = file.read()

    # 使用 re.DOTALL 标志来匹配多行直到分号
    pattern = r'typedef\s+(\w+\s*\**)\s*\(\*([a-zA-Z_][a-zA-Z0-9_]*_api_ptr_t)\)\((.*?)\);'

    # 使用 re.DOTALL 标志
    function_pointer_types = re.findall(pattern, content, re.DOTALL)

    # 生成 C 语言实现
    with open(rt_api_c_file, 'w') as file:
        # 写入头文件包含
        file.write('#include "common.h"\n')
        file.write('#include <stdarg.h>\n')
        file.write('#include "rt_api_addr.h"\n')
        file.write('#include "rt_api_typedef.h"\n')
        file.write('#include "rttypes.h"\n')
        file.write('#include <stddef.h>\n\n')

        for return_type, func_type, params in function_pointer_types:
            # 构建函数实现
            func_name = func_type[:-10]  # 去掉 "_api_ptr_t"
            
            # 优化参数名提取逻辑
            param_names = []
            for param in params.split(','):
                param = param.strip()
                if '*' in param and '(' in param:  # 处理形如 `void (*entry)(void *parameter)` 的参数
                    # 正确提取参数名
                    param_name = param.split('(')[1].split(')')[0].split('*')[-1].strip()
                else:
                    param_name = param.split('*')[-1].split()[-1]
                    if param_name == 'void':
                        param_name = ''
                param_names.append(param_name)
            
            param_names_str = ', '.join(param_names)
            
            func_def = f'{return_type} {func_name}({params}) {{\n'
            if func_name == 'rt_kprintf':
                func_impl = """#if APP_LOG_BUF_SIZE
    va_list args;
    int len;
    static char rt_kprintf_app_buf[APP_LOG_BUF_SIZE];

    va_start(args, fmt);
    rt_vsnprintf(rt_kprintf_app_buf, sizeof(rt_kprintf_app_buf) - 1, fmt, args);
    len = ((rt_kprintf_api_ptr_t)(rt_kprintf_addr))("%s", rt_kprintf_app_buf);
    va_end(args);

    return len;
#else
    return 0;
#endif
}\n
"""
            else:
                func_impl = f'    return (({func_type})({func_name}_addr))({param_names_str});\n}}\n\n'
            func_def += func_impl
            
            # 写入文件
            file.write(func_def)
