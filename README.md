# build rdma client/server

```bash

g++ rdmaServer.c -lrdmacm -libverbs -o rs.out

g++ rdmaClient.c -lrdmacm -libverbs -o rc.out

```

# build tcp client/server

```bash

g++ tcpServer.c -o s.out

g++ tcpClient.c -o c.out

```