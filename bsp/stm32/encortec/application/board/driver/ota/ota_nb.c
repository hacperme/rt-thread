/*
 * @FilePath: ota_nb.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-11-16 13:50:41
 * @copyright : Copyright (c) 2024
 */
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include "rtthread.h"
#include "rtdevice.h"
#include "logging.h"
#include "upgrade_manager.h"
#include "tools.h"
#include "board.h"
#include "at.h"
#include "lpm.h"

void nb_ota_prepare(void *node)
{
    UpgradeNode *_node = (UpgradeNode *)node;
    _node->status = UPGRADE_STATUS_PREPARED;

}

void nb_ota_apply(int* progress, void *node)
{
    UpgradeNode *_node = (UpgradeNode *)node;
    _node->status = UPGRADE_STATUS_SUCCESS;
}

void nb_ota_finish(void *node)
{

}

UpgradeModuleOps nb_ota_ops = {
    .prepare = nb_ota_prepare,
    .apply = nb_ota_apply,
    .finish = nb_ota_finish,
};
