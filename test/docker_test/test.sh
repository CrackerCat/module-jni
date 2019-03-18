#!/bin/bash

set -e
set -x

export ENV_FILE=/tmp/env.sh

. ${ENV_FILE}

# setup MODULE_SRC_DIR env var
cwd=`pwd`
if [ "${MODULE_SRC_DIR}" = "" ]; then
    if [ -e "$cwd/src/jni-module.cpp" ]; then
        MODULE_SRC_DIR=$cwd
    else
        MODULE_SRC_DIR=$WORKDIR/module-jni
    fi
fi
echo "export MODULE_SRC_DIR=${MODULE_SRC_DIR}" >> ${ENV_FILE}

echo "export QORE_UID=999" >> ${ENV_FILE}
echo "export QORE_GID=999" >> ${ENV_FILE}

echo "export GLASSFISH_USER=qore" >> ${ENV_FILE}
echo "export GLASSFISH_HOME=/home/qore/glassfish4" >> ${ENV_FILE}
echo "export GLASSFISH_JAR=\${GLASSFISH_HOME}/glassfish/lib/gf-client.jar" >> ${ENV_FILE}

. ${ENV_FILE}

export MAKE_JOBS=4

# build module and install
echo && echo "-- building module --"
cd ${MODULE_SRC_DIR}
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=debug -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}
make -j${MAKE_JOBS}
make install

# add Qore user and group
groupadd -o -g ${QORE_GID} qore
useradd -o -m -d /home/qore -u ${QORE_UID} -g ${QORE_GID} qore

# download glassfish
cd /home/qore
git clone -b master --single-branch --depth 1 https://gitlab+deploy-token-7:eQb7E4YBX9jzcv6BFNog@git.qoretechnologies.com/infrastructure/glassfish4.git
rm -rf glassfish4/.git

# own everything by the qore user
chown -R qore:qore ${MODULE_SRC_DIR} /home/qore

# start glassfish
echo && echo "-- starting Glassfish --"
gosu qore:qore $GLASSFISH_HOME/bin/asadmin start-domain domain1
sleep 2

# create glassfish queue named abc, needed for the tests
gosu qore:qore $GLASSFISH_HOME/bin/asadmin create-jms-resource --restype javax.jms.Queue abc

# run the tests
export QORE_MODULE_DIR=${MODULE_SRC_DIR}/qlib:${QORE_MODULE_DIR}
cd ${MODULE_SRC_DIR}
for test in test/*.qtest; do
    gosu qore:qore qore $test -vv
done