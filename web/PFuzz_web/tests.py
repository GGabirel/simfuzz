# Create your tests here.

from PFuzz_web.models import *
from mongoengine.context_managers import switch_db
from mongoengine.context_managers import switch_collection
import os
import struct
def main():

    en = ReadDB()
    print(en.get_db_list())
    # data = get_binary_by_name('binary', 'lz4_1').get_data().code
    # BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    # destination = os.path.join(BASE_DIR, 'static', 'files', 'binary.bin')
    # if os.path.exists(destination):
    #     os.remove(destination)
    # with open(destination, 'w+') as dest:
    #     # binary_data = struct.unpack('B',data)
    #     dest.write(str(data))
    #     dest.close()
    #     print('destination')
    #     print (dest)
    #     return dest

    # resone = en.get_more()
    # print ("get one")
    # print (resone)
    #
    # count = en.get_count()
    # print ("count")
    # print (count)

    # en1 = get_binary_by_name('binary','lz4_1').get_data()
    # print(en1.name)
    # print('hello')

if __name__ == "__main__":
    main()