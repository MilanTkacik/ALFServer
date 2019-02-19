# ALFServer

## Instalation

download the project and enter alfserver directory

cmake3 . -DBOOST_INCLUDEDIR=/opt/o2-dependencies/include -DBOOST_LIBRARYDIR=/opt/o2-dependencies/lib

make all

## Run ALFServer

DIM_DNS_NODE=aldcs012 CRU_TABLE_PATH=config/cru_table.py CRU_LIST_PATH=config/cru_list.conf ./bin/ALFServer

where:

DIM_DNS_NODE - domain name of machine running DIM DNS

CRU_TABLE_PATH - path to the python file containing the CRU registers addresses, depends on CRU firmware version

CRU_LIST_PATH - path to the file containing list PCIe addresses of connected CRUs, must be modified according to machine configuration