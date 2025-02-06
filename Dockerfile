# NOTE: You might have to change this distribution or version depending on where the task-tracker executable was compiled
FROM fedora:41

RUN dnf -y update
RUN dnf -y install libmicrohttpd

COPY configuration.json server.crt server.key task-trackerd /root/task-tracker/
COPY resources/ /root/task-tracker/resources/

EXPOSE 8443/tcp

WORKDIR /root/task-tracker/

ENTRYPOINT /root/task-tracker/task-trackerd
