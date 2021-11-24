# build rdma client/server

```bash

g++ rdmaServer.c -lrdmacm -libverbs -o rs.out

g++ rdmaClient.c -lrdmacm -libverbs -o rc.out

```

## build rdma client/server using local rdma lib path

```bash

export  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:{path-to-local-lib}

g++ rdmaServer.c -lrdmacm -libverbs -L{path-to-local-lib} -o rs.out

g++ rdmaClient.c -lrdmacm -libverbs -L{path-to-local-lib}  -o rc.out

```

# build tcp client/server

```bash

g++ tcpServer.c -o s.out

g++ tcpClient.c -o c.out

```