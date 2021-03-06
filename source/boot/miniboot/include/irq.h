/******************************************************************************
 *    Copyright (C) 2014 Hisilicon STB Development Dept
 *    All rights reserved.
 * ***
 *    Create by Czyong
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *   http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
******************************************************************************/

#ifndef IRQH
#define IRQH
/******************************************************************************/

#include <stddef.h>
#include <cpu-arch.h>

void irq_init(void);

int irq_connect(uint32 irqnr, void (*handle)(void *), void *arg);

void irq_handle(uint32 irqnr);

/******************************************************************************/
#endif /* IRQCHIPH */
