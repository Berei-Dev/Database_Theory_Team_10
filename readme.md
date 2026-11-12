 # Database Theory Team 10

## Docker setup

- Docker version: [4.41.2 (191736)]
- Docker Compose version: [4.41.2 (191736)]
- Compose file path: [/docker]
- PostgreSQL service / container name: [team10-postgres]
- Custom PostgreSQL image tag or build context: [:custom]
- Database name and username: [imbd]
- Host-to-container port mapping: [5432:5432]
- Persistent volume name / mount: [postgres_data:/var/lib/postgresql]
- Docker network: [team10-net]
- SSH service / container and published port: [2222]
- pgAdmin service / container and published port: [8080]

## Build the custom Docker PostgreSQL implementation

Build the image from the project Dockerfile. Any changes done to /docker/src/backend/executor/nodeHash.c OR nodeHashjoin.c will be reflected in the build

```sh
docker build -f ~/docker -t team10-postgres:custom .
```

## Start Custom Docker PostgreSQL implementation

Create a new container on every boot that uses the same background information:

```sh
docker run --rm --name team-10-postgres --network team10-net --shm-size=512m -e POSTGRES_PASSWORD=de42573a -p 5432:5432 -v postgres_data:/var/lib/postgresql -d team10-postgres:custom
```

## Create the live pgAdmin container

Create the pgAdmin container:

```sh
docker run -d --name pgadmin4 --network team10-net -p 8080:80 -e PGADMIN_DEFAULT_EMAIL=<username> -e PGADMIN_DEFAULT_PASSWORD=<password> -v pgadmin-data:/var/lib/pgadmin dpage/pgadmin4
```

## Create the SSH container

Connect all users to your choice of VPN over the same network. In my case, I utilize my university's VPN.

Start the SSH service:

```sh
docker run -d --name ssh-server --network team10-net -p 2222:22 --restart unless-stopped alpine:latest sh -c "apk add --no-cache openssh && ssh-keygen -A && adduser -D -s /bin/sh teamuser && passwd -u teamuser && echo 'PermitRootLogin no' > /etc/ssh/sshd_config && echo 'PubkeyAuthentication yes' >> /etc/ssh/sshd_config && echo 'PasswordAuthentication no' >> /etc/ssh/sshd_config && echo 'AllowTcpForwarding yes' >> /etc/ssh/sshd_config && echo 'X11Forwarding no' >> /etc/ssh/sshd_config && echo 'AllowAgentForwarding yes' >> /etc/ssh/sshd_config && mkdir -p /home/teamuser/.ssh && echo '<ssh key 1>' > /home/teamuser/.ssh/authorized_keys && echo '<ssh key 2>' >> /home/teamuser/.ssh/authorized_keys && echo '<ssh key 3>' >> /home/teamuser/.ssh/authorized_keys && echo '<ssh key 4>' >> /home/teamuser/.ssh/authorized_keys && chown -R teamuser:teamuser /home/teamuser/.ssh && chmod 700 /home/teamuser/.ssh && chmod 600 home/teamuser/.ssh/authorized_keys && exec /usr/sbin/sshd -D -e"
```

Connect to the SSH container (replace the docker-host with the host's current VPN IP Address):

```sh
ssh -p 2222 teamuser@[docker-host]
```

Configure the local database client to connect to `localhost:[local-port]`. When the containers share a Docker network, `[postgres-host-from-ssh-container]` is usually the PostgreSQL Compose service name. Use the project's database name and credentials.

## Access the Databases via the SSH tunnel and the pgAdmin container

After all containers are started and everyone is connected to the same network via a VPN, Open pgAdmin at `http://[docker-host]:[published-pgadmin-port]` and sign in with the configured pgAdmin account. In **Add New Server**, enter a display name, then set the connection details:

- **Host name/address:** PostgreSQL Compose service name (if on the same Docker network)
- **Port:** PostgreSQL container port
- **Maintenance database:** [database name]
- **Username:** [database username]
- **Password:** Retrieve from the configured secret or `.env` file; do not record it here.

pgAdmin URL: [http://localhost:8080]  



