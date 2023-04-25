#
#  Original code (C) Copyright Renesas Electronics Corporation 2023
#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

import glob
import os

dir = os.path.dirname(__file__)
py_list = glob.glob(dir+'/*.py')
py_list.remove(dir+'/__init__.py')
def cut_py(file_path):
    file_path = file_path.replace('.py', '')
    file_path = file_path.replace(dir+'/', '')
    return file_path
__all__ = list(map(cut_py, py_list))