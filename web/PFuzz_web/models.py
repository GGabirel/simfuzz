from django.db import models

# Create your models here.
import pymongo
from mongoengine import *
from mongoengine.context_managers import switch_db
from django.utils import timezone

serverDB = pymongo.MongoClient("mongodb://127.0.0.1:27017/")

connect('lz4_1',host='192.168.252.7',port=27017,alias='lz4_1')
connect('mp3gain1-6-2',host='192.168.252.7',port=27017,alias='mp3gain1-6-2')
connect('sam2p0-49-4',host='192.168.252.7',port=27017,alias='sam2p0-49-4')
connect('unrar5-7-1',host='192.168.252.7',port=27017,alias='unrar5-7-1')

class SeedBook(Document):
    len = IntField()
    fname = StringField()
    meta = {'db_alias': 'lz4_1'}

class binary(Document):
    name = StringField()
    code = BinaryField()
    length = IntField()
    info = StringField()
    meta = {'db_alias': 'lz4_1'}

class crashes(Document):
    fname = StringField()
    seed = BinaryField()
    len = IntField()
    describe = StringField()
    time = IntField()
    meta = {'db_alias':'lz4_1'}

class seed(Document):
    fname = StringField()
    seed = BinaryField()
    len = IntField()
    describe = StringField()
    time = IntField()
    havoc_time = IntField()
    meta = {'db_alias': 'lz4_1'}

class virgin_bits(Document):
    time = IntField()
    bitmap = BinaryField()
    flag = IntField()
    meta = {'db_alias': 'lz4_1'}

class virgin_crash(Document):
    bitmap = BinaryField()
    flag = IntField()
    time = IntField()
    meta = {'db_alias': 'lz4_1'}

class virgin_tmout(Document):
    bitmap = BinaryField()
    flag = IntField()
    time = IntField()
    meta = {'db_alias': 'lz4_1'}


class TestMongoEngine(object):
    def __init__(self, colname,):
        colname = eval(colname)
        self.colname = colname
        # self.dbname = dbname

    def get_one(self):
        return self.colname.objects.first()

    def get_more(self):
        return list(self.colname.objects.all())

    def get_count(self):
        return self.colname.objects.count()

class TestMoreMongo(Document):

    def change_db(self, db_name,colname):
        with switch_db(eval(colname), db_name) as col:
            return col.objects.count()  # Saves the 'archive-user-db'

class ChangeDB(Document):

    def get_count(self, db_name,colname):
        with switch_db(eval(colname), db_name) as col:
            return col.objects.count()  # Saves the 'archive-user-db'

    def get_all(self, db_name, colname):
        with switch_db(eval(colname), db_name) as col:
            return col.objects.all()


class get_binary_by_name(object):
    def __init__(self, colname,filename):
        colname = eval(colname)
        self.colname = colname
        self.filename = filename

    def get_data(self):
        data = self.colname.objects(name=self.filename)
        return data[0]

class create_db(Document):
    def create_linkdb(self,data):
        link_db=serverDB['Task_Relation']
        for key,value in data.items():
            collection = link_db[str(key)]
            collection.insert_many(value)
        return '0'

class update_db(Document):
    def update_linkdb(self,data):
        link_db = serverDB['Task_Relation']
        for key,value in data.items():
            collection = link_db[str(key)]
            for v in range(0, len(value)):
                collection.update({"task_name":{"$eq":value[v]["task_name"]}},{"$set":value[v]})

class ReadDB(Document):
    def get_db_list(self):
        print("======%%%%%=====dblist==start:" + timezone.localtime(timezone.now()))
        dblist = serverDB.list_database_names()
        print("======%%%%%=====dblist==stop:" + timezone.localtime(timezone.now()))
        return dblist

    def get_count(self, db_name, colname):
        db = serverDB[db_name]
        col_list = db.list_collection_names()
        if colname in col_list:
            col_count = db[colname].count()
            return col_count
        else:
            return "0"

    def get_all(self, db_name, colname):
        db = serverDB[db_name]
        col = db[colname]
        return col.find()

    def get_all_col(self, db_name):
        db = serverDB[db_name]
        col=db.collection_names(include_system_collections=False)
        return col

    def get_data_by_col(self,db_name,col_name):
        db = serverDB[db_name]
        results = db[col_name].find()
        return results

    def get_name_by_value(self,db_name, colname, name,value):
        db = serverDB[db_name]
        col = db[colname]
        print (value)
        query = {name:value}
        print(col.find(query))
        return col.find(query)

    def get_file_name(self,db_name):
        db = serverDB[db_name]
        results = db['binary'].find()
        for res in results:
            return res['name']

    def add_info_file(self, db_name, fname, value):
        db = serverDB[db_name]
        col = db['binary']
        query = {'name':fname}
        print(db_name)
        print(value)
        newvalue = {"$set":{"info": value}}
        res = col.update(query, newvalue)
        print(res)
        return res

    def add_info_linkdb(self, col_name, task_name, value):
        db = serverDB["Task_Relation"]
        col = db[col_name]
        query = {'task_name':task_name}
        newvalue = {"$set":{"describe": value}}
        res = col.update(query, newvalue)
        return res

    def add_url_linkdb(self, col_name, task_name, value):
        db = serverDB["Task_Relation"]
        col = db[col_name]
        query = {'task_name':task_name}
        newvalue = {"$set":{"url": value}}
        res = col.update(query, newvalue)
        return res

    def add_addr_file(self, db_name, fname, value):
        db = serverDB[db_name]
        col = db['binary']
        query = {'name':fname}
        print(db_name)
        print(fname)
        newvalue = {"$set":{"address":value}}
        res = col.update(query,newvalue)

        return res

    def get_info(self, db_name):
        db = serverDB[db_name]
        results = db['binary'].find()
        for res in results:
            if 'info' in res:
                return res['info']
            else:
                return "暂无描述信息"

    def get_address(self, db_name):
        db = serverDB[db_name]
        results = db['binary'].find()
        for res in results:
            if 'address' in res:
                return res['address']
            else:
                return "http://xxx.xxx.xx/"

    def remove_task(self, db_name):
        db = serverDB[db_name]
        ret = db.dropDatabase()
        print(ret)
        return ret







































