from PFuzz_web.models import *
from django.shortcuts import render, HttpResponse
from django.forms.models import model_to_dict
from django.shortcuts import HttpResponseRedirect,Http404,render_to_response
from django.http import JsonResponse
from django.http import JsonResponse
from django.db.models import Q
from django.core import serializers
from django.utils.functional import curry
from PFuzz_web.models import binary
from django.views.decorators.csrf import csrf_exempt
from django.core.paginator import Paginator

import os
import json
from PFuzz_web import models
import numpy as np

def get_data(request):
    if request.method == "GET":
        db_name = request.GET['db']
        col_name = request.GET['col']
        print(col_name)
        rd = ReadDB()
        result = rd.get_all(db_name, col_name)
        print(result)
        return render_to_response( "data_detailes.html", {'result':result,'db_name':db_name, 'col_name':col_name})
    # result = ''
    if request.is_ajax():
        db_name = request.POST.get('db_name')
        col_name = request.POST.get('col_name')
        rd = ReadDB()
        result = rd.get_all(db_name, col_name)
        print(result)
        return JsonResponse(result)

@csrf_exempt
def index(request):

    rd = ReadDB()
    if request.is_ajax():
        db_name = request.POST.get('db_name')
        total_crash = rd.get_count(db_name,'crashes')
        seed_count = rd.get_count(db_name,'seed')
        file_name = rd.get_file_name(db_name)
        describe = rd.get_info(db_name)
        address =rd.get_address(db_name)
        response = JsonResponse({"address":address,"describe":describe,"total_crash": total_crash, "seed_count":seed_count, "filename":file_name})
        return response

    task_relation={}
    collections = rd.get_all_col("Task_Relation")
    for col in collections:
        lists = rd.get_data_by_col("Task_Relation", col)
        task_relation[col] = []
        for list in lists:
            total_crash = rd.get_count(list["task_name"], 'crashes')
            seed_count = rd.get_count(list["task_name"], 'seed')
            if "describe" in list.keys():
                task_relation[col].append({"task_name":list["task_name"],"total_crash":total_crash, "seed_count":seed_count,"url":list["url"],"describe":list["describe"]})
            else:
                task_relation[col].append({"task_name":list["task_name"],"total_crash":total_crash, "seed_count":seed_count,"url":list["url"],"describe":"NULL"})
    context={"task_relation":task_relation}
    print(task_relation)
    return render(request, "index.html", context)

@csrf_exempt
def newIndex(request):

    rd = ReadDB()
    task_relation=[]
    collections = rd.get_all_col("Task_Relation")
    for col in collections:
        if col not in task_relation:
            task_relation.append(col)
    context = {"task_relation":task_relation}
    return render(request, "newIndex.html", context)


@csrf_exempt
def get_task_details(request):
    rd = ReadDB()
    if request.is_ajax():
        program_name = request.POST.get('program_name')
        tasks = rd.get_data_by_col('Task_Relation', program_name)
        print("printing..."+program_name)
        count = rd.get_count('Task_Relation', program_name)
        print('count:'+str(count))
        task_list={}
        for task in tasks:
            task_name = task['task_name']
            total_crash = rd.get_count(task_name, 'crashes')
            seed_count = rd.get_count(task_name, 'seed')
            task_list[task_name] = []
            if "describe" in task.keys():
                print('pppp')
                task_list[task_name].append(
                    {"total_crash": total_crash, "seed_count": seed_count,
                     "url": task["url"], "describe": task["describe"]})
            else:
                print('bbbb')
                task_list[task_name].append(
                    {"total_crash": total_crash, "seed_count": seed_count,
                     "url": task["url"], "describe": "NULL"})
        response = JsonResponse(task_list)

        # task_relation = {}
        # lists = rd.get_data_by_col("Task_Relation", program_name)
        # task_relation[program_name] = []
        # for list in lists:
        #     total_crash = rd.get_count(list["task_name"], 'crashes')
        #     seed_count = rd.get_count(list["task_name"], 'seed')
        #     if "describe" in list.keys():
        #         task_relation[program_name].append(
        #             {"task_name": list["task_name"], "total_crash": total_crash, "seed_count": seed_count,
        #              "url": list["url"], "describe": list["describe"]})
        #     else:
        #         task_relation[program_name].append(
        #             {"task_name": list["task_name"], "total_crash": total_crash, "seed_count": seed_count,
        #              "url": list["url"], "describe": "NULL"})
        # response = JsonResponse(task_relation)
        return response


@csrf_exempt
def get_linkdb(request):
    rd = ReadDB()
    if request.is_ajax():
        db_list = rd.get_db_list()
        lists = []
        new_dbs = {}
        for db in db_list:
            if db != 'admin' and db != 'config' and db != 'db_name' and db != 'local' and db!='Task_Relation':
                lists.append(db)
        for item in lists:
            file_name = rd.get_file_name(item)
            describe = rd.get_info(item)

            if file_name in new_dbs.keys():
                new_dbs[file_name].append({'task_name': item, 'describe': describe})
            else:
                new_dbs[file_name] = [{'task_name': item, 'describe': describe}]
        response = JsonResponse(new_dbs)
        return response


@csrf_exempt
def get_link_db(request):
    rd = ReadDB()
    col_dic = {}
    if request.is_ajax():
        collections = rd.get_all_col("Task_Relation")
        for col in collections:
            lists = rd.get_data_by_col("Task_Relation", col)
            col_dic[col]=[]
            for list in lists:
                if "describe" in list.keys():
                    col_dic[col].append({"task_name":list["task_name"],"total_crash":list["total_crash"], "seed_count":list["seed_count"],"url":list["url"],"describe":list["describe"]})
                else:
                    col_dic[col].append({"task_name":list["task_name"],"total_crash":list["total_crash"], "seed_count":list["seed_count"],"url":list["url"],"describe":"NULL"})

        response = JsonResponse(col_dic,safe=False)
        return response

@csrf_exempt
def update_des(request):
    if request.is_ajax():
        print("describe")
        db_name = request.POST.get('db_name')
        fname = request.POST.get('filename')
        describe = request.POST.get('describe')
        rd = ReadDB()
        res = rd.add_info_file(db_name,fname,describe)
        print("describe")
        return JsonResponse({"result":"success"})

def test(request):
    en = TestMoreMongo()
    return render(request,"testDB.html",{"binary":en})

@csrf_exempt
def load_binary(request):
    if request.is_ajax():
        db_name = request.POST.get('db')
        col_name = request.POST.get('col')
        id = request.POST.get('id')
        id = eval(repr(id).replace(' ', '+'))
        filename = db_name + '_' + eval(repr(id).replace('/', '_')) + '.' + col_name
        rd = ReadDB()
        results = rd.get_name_by_value(db_name, col_name, 'fname', str(id))

        print('hahah')
        for result in results:
            data = result['seed']
            BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
            destination = os.path.join(BASE_DIR, 'static', 'files', filename)
            if os.path.exists(destination):
                os.remove(destination)
            with open(destination, 'wb+') as dest:
                dest.write(data)
                dest.close()
                print(dest)
            return JsonResponse({'filename':filename})


def task_profile(request):
    return render(request, "task_profile.html")


# 读取程序列表
@csrf_exempt
def get_programs_list(request):
    if request.method == "POST":
        rd = ReadDB()
        program_list = []
        collections = rd.get_all_col("Task_Relation")
        for col in collections:
            tasksnum = rd.get_count('Task_Relation', col)
            tasks = rd.get_data_by_col('Task_Relation', col)
            url = 'http://xxx.xxx.xx'
            for task in tasks:
                url = task['url']
                break
            program_list.append({"name": col, "tasksNum": tasksnum, "url": url, "operation": "o"})
        context = {"dataList": program_list}
        return JsonResponse(context)
    return render(request, "home.html")

# 读取程序下对应任务列表
@csrf_exempt
def get_task_list(request):
    rd = ReadDB()
    program_name = request.GET.get("name")
    tasks = rd.get_data_by_col('Task_Relation', program_name)
    task_list = []
    for task in tasks:
        task_name = task['task_name']
        total_crash = rd.get_count(task_name, 'crashes')
        seed_count = rd.get_count(task_name, 'seed')
        if "describe" in task.keys():
            task_list.append(
                {"task_name": task_name, "crashesNum": total_crash, "seedsNum": seed_count,
                 "url": task["url"], "describe": task["describe"], "operation": "o"})
        else:
            task_list.append(
                {"task_name": task_name, "crashesNum": total_crash, "seedsNum": seed_count,
                 "url": task["url"], "describe": "NULL"})
    result = {"row": task_list}
    return JsonResponse(result)


# 更新数据库描述
@csrf_exempt
def update_linkdb_des(request):
    if request.is_ajax():
        task_name = request.POST.get('task_name')
        col_name = request.POST.get('program_name')
        describe = request.POST.get('describe')
        rd = ReadDB()
        res = rd.add_info_linkdb(col_name,task_name,describe)
        res = rd.add_info_file(task_name,col_name, describe)
        return JsonResponse({"result":"success"})

# 更新任务链接
@csrf_exempt
def update_addr(request):
    program_name = request.POST.get('program_name')
    address = request.POST.get('url')
    rd = ReadDB()
    tasks = rd.get_data_by_col('Task_Relation', program_name)
    for task in tasks:
        print("task:url")
        print(task['url'])
        print(address)
        task_name = task['task_name']
        res = rd.add_url_linkdb(program_name, task_name, address)
        print(res)
    return JsonResponse({"result": "success"})


# 删除程序 分别删除关系表中的collection和该程序下所有任务数据库
@csrf_exempt
def delete_program(request):
    if request.is_ajax():
        program_name = request.POST.get('program_name')
        db = serverDB['Task_Relation']
        rd = ReadDB()
        tasks = rd.get_data_by_col('Task_Relation', program_name)
        for task in tasks:
            task_name = task['task_name']
            db_task = serverDB[task_name]
            res = db_task.dropDatabase()

        db_collection = db[program_name]
        db_collection.drop()

        return JsonResponse({"result":"success"})

# 删除任务 (分别删除该程序collection下该条任务记录和该任务数据库)
@csrf_exempt
def delete_task(request):
    if request.is_ajax():
        task_name = request.POST.get('task_name')
        program_name = request.POST.get('program_name')
        db = serverDB['Task_Relation']
        db_collection = db[program_name]
        query = {"task_name": task_name}
        db_collection.remove_many(query)
        db_task = serverDB[task_name]
        db_task.dropDatabase()
        return JsonResponse({"result":"success"})

@csrf_exempt
def get_chart_data(request):
    if request.is_ajax():
        task_id = request.POST.get('id')
        task_name = request.POST.get('name')
        return JsonResponse({"result":"success"})
