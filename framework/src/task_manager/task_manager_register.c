/****************************************************************************
 *
 * Copyright 2018 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <task_manager/task_manager.h>
#include "task_manager_internal.h"

/****************************************************************************
 * task_manager_register
 ****************************************************************************/
int task_manager_register(char *name, int permission, int timeout)
{
	int status;
	tm_request_t request_msg;
	tm_response_t response_msg;

	if (name == NULL || timeout < TM_RESPONSE_WAIT_INF || permission < TM_TASK_PERMISSION_ALL || permission > TM_TASK_PERMISSION_DEDICATE) {
		return TM_INVALID_PARAM;
	}

	memset(&request_msg, 0, sizeof(tm_request_t));
	request_msg.cmd = TASKMGT_REGISTER_TASK;
	request_msg.caller_pid = getpid();
	request_msg.handle = permission;
	request_msg.data = (void *)TM_ALLOC(strlen(name) + 1);
	if (request_msg.data == NULL) {
		return TM_OUT_OF_MEMORY;
	}
	strncpy((char *)request_msg.data, name, strlen(name) + 1);
	request_msg.timeout = timeout;

	if (timeout != TM_NO_RESPONSE) {
		asprintf(&request_msg.q_name, "%s%d", TM_PRIVATE_MQ, request_msg.caller_pid);
	}

	status = taskmgr_send_request(&request_msg);
	if (status < 0) {
		return TM_FAIL_REQ_TO_MGR;
	}

	if (timeout != TM_NO_RESPONSE) {
		status = taskmgr_receive_response(request_msg.q_name, &response_msg, timeout);
		TM_FREE(request_msg.q_name);
		if (status == OK) {
			status = response_msg.status;
		}
	}

	return status;
}
