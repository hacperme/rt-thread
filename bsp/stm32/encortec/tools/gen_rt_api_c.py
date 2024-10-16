import re


def get_param_names(params):
    param_names = []
    end_index = params.find(",")
    param_item = params if end_index == -1 else params[:end_index]
    param_item = param_item.strip()
    if param_item.find("(") != -1 and param_item.find("*") != -1:
        while param_item.count("(") != param_item.count(")") and end_index != -1:
            end_index = params[end_index + 1:].find(",")
            param_item = params if end_index == -1 else params[:end_index]
        if end_index == -1 and param_item.count("(") != param_item.count(")"):
            raise ValueError("Param[%s] is invalid!!!!" % param_item)
        param_name = param_item.split('(')[1].split(')')[0].split('*')[-1].strip()
    else:
        param_name = param_item.split('*')[-1].split()[-1]
        if param_name == 'void':
            param_name = ''
        if param_name.find("[") != -1:
            param_name = param_name.split("[")[0]
    param_names.append(param_name)
    if end_index != -1:
        param_names += get_param_names(params[end_index + 1:])
    return param_names


def gen_rt_api_c(rt_api_typedef_h_file, rt_api_c_file):

    # 读取头文件内容
    with open(rt_api_typedef_h_file, 'r') as file:
        content = file.read()

    # 使用 re.DOTALL 标志来匹配多行直到分号
    pattern = r'typedef\s+(\w+\(*\(*\w*\)*\)*\s*\w*\s*\w*\s*\**)\s*\(\*([a-zA-Z_][a-zA-Z0-9_]*_api_ptr_t)\)\((.*?)\);'

    # 使用 re.DOTALL 标志
    function_pointer_types = re.findall(pattern, content, re.DOTALL)

    # 生成 C 语言实现
    with open(rt_api_c_file, 'w') as file:
        # 写入头文件包含
        file.write('#include "common.h"\n')
        file.write('#include <stdio.h>\n')
        file.write('#include <stdarg.h>\n')
        file.write('#include <stddef.h>\n')
        file.write('#include "rt_api_addr.h"\n')
        file.write('#include "rt_api_typedef.h"\n')
        file.write('#include "rttypes.h"\n')
        file.write('#include "logging.h"\n')
        file.write('#include "stm32u5xx_hal.h"\n')
        file.write('#include "ota_app.h"\n')
        file.write('#include <sys/stat.h>\n\n')

        for return_type, func_type, params in function_pointer_types:
            # 构建函数实现
            func_name = func_type[:-10]  # 去掉 "_api_ptr_t"

            # 优化参数名提取逻辑
            param_names = []
            param_names = get_param_names(params)
            # for param in params.split(','):
            #     param = param.strip()
            #     if '*' in param and '(' in param:  # 处理形如 `void (*entry)(void *parameter)` 的参数
            #         # 正确提取参数名
            #         param_name = param.split('(')[1].split(')')[0].split('*')[-1].strip()
            #     else:
            #         param_name = param.split('*')[-1].split()[-1]
            #         if param_name == 'void':
            #             param_name = ''
            #     param_names.append(param_name)

            param_names_str = ', '.join(param_names)

            return_type = return_type.strip()
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
            elif func_name == "at_obj_exec_cmd":
                func_impl = """    va_list args;
    rt_err_t result = RT_EOK;

    RT_ASSERT(cmd_expr);

    if (client == RT_NULL)
    {
        log_error("input AT Client object is NULL, please create or get AT Client object!");
        return -RT_ERROR;
    }

    /* check AT CLI mode */
    if (client->status == AT_STATUS_CLI && resp)
    {
        return -RT_EBUSY;
    }

    rt_mutex_take(client->lock, RT_WAITING_FOREVER);

    client->resp_status = AT_RESP_OK;

    if (resp != RT_NULL)
    {
        resp->buf_len = 0;
        resp->line_counts = 0;
    }

    client->resp = resp;
    rt_sem_control(client->resp_notice, RT_IPC_CMD_RESET, RT_NULL);

    va_start(args, cmd_expr);
    client->last_cmd_len = at_vprintfln(client->device, client->send_buf, client->send_bufsz, cmd_expr, args);
    if (client->last_cmd_len > 2)
    {
        client->last_cmd_len -= 2; /* \"\\r\\n\" */
    }
    va_end(args);

    if (resp != RT_NULL)
    {
        if (rt_sem_take(client->resp_notice, resp->timeout) != RT_EOK)
        {
            log_warn("execute command (%.*s) timeout (%d ticks)!", client->last_cmd_len, client->send_buf, resp->timeout);
            client->resp_status = AT_RESP_TIMEOUT;
            result = -RT_ETIMEOUT;
        }
        else if (client->resp_status != AT_RESP_OK)
        {
            log_error("execute command (%.*s) failed!", client->last_cmd_len, client->send_buf);
            result = -RT_ERROR;
        }
    }

    client->resp = RT_NULL;

    rt_mutex_release(client->lock);

    return result;\n}\n
"""
            elif func_name == "at_resp_parse_line_args":
                func_impl = """    va_list args;
    int resp_args_num = 0;
    const char *resp_line_buf = RT_NULL;

    RT_ASSERT(resp);
    RT_ASSERT(resp_expr);

    if ((resp_line_buf = at_resp_get_line(resp, resp_line)) == RT_NULL)
    {
        return -1;
    }

    va_start(args, resp_expr);

    resp_args_num = vsscanf(resp_line_buf, resp_expr, args);

    va_end(args);

    return resp_args_num;\n}\n
"""
            elif func_name == "at_resp_parse_line_args_by_kw":
                func_impl = """    va_list args;
    int resp_args_num = 0;
    const char *resp_line_buf = RT_NULL;

    RT_ASSERT(resp);
    RT_ASSERT(resp_expr);

    if ((resp_line_buf = at_resp_get_line_by_kw(resp, keyword)) == RT_NULL)
    {
        return -1;
    }

    va_start(args, resp_expr);

    resp_args_num = vsscanf(resp_line_buf, resp_expr, args);

    va_end(args);

    return resp_args_num;\n}\n
"""
            elif func_name == "rt_sprintf":
                func_impl = """    rt_int32_t n = 0;
    va_list arg_ptr;

    va_start(arg_ptr, format);
    n = rt_vsprintf(buf, format, arg_ptr);
    va_end(arg_ptr);

    return n;\n}\n
"""
            elif func_name == "rt_snprintf":
                func_impl = """    rt_int32_t n = 0;
    va_list args;

    va_start(args, format);
    n = rt_vsnprintf(buf, size, format, args);
    va_end(args);

    return n;\n}\n
"""
            elif func_name == "_exit":
                func_impl = f'    (({func_type})({func_name}_addr))({param_names_str});\n}}\n\n'
            else:
                func_impl = f'    return (({func_type})({func_name}_addr))({param_names_str});\n}}\n\n'
            func_def += func_impl

            # 写入文件
            file.write(func_def)
