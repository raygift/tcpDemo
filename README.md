# build rdma client/server

```bash

g++ rdmaServer.c -lrdmacm -libverbs -o rs.out

g++ rdmaClient.c -lrdmacm -libverbs -o rc.out

```

## build rdma client/server using local rdma lib path

```bash

# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/vm1/Desktop/CProgram/rdma-core/build/lib

g++ rdmaServer.c -lrdmacm -libverbs -L/home/vm1/Desktop/CProgram/rdma-core/build/lib -Wl,-rpath,/home/vm1/Desktop/CProgram/rdma-core/build/lib -o rs.out

g++ rdmaClient.c -lrdmacm -libverbs -L/home/vm1/Desktop/CProgram/rdma-core/build/lib -Wl,-rpath,/home/vm1/Desktop/CProgram/rdma-core/build/lib -o rc.out

```

# build tcp client/server

```bash

g++ tcpServer.c -o s.out

g++ tcpClient.c -o c.out

```