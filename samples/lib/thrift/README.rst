.. _sockets-echo-sample:

Hello Sample Application
########################

Overview
********

This sample application includes a client a server implementing the RPC 
interface described in ``thrift/hello.thrift``. The purpose of this 
example is to demonstrate how components at different layer in thrift can
be combined to build an application with desired features. 


Requirements
************

- :ref:`networking_with_host`
- a Linux host with thrift installed

Building and Running
********************

This application can be run on a Linux host, with either the server or the
client in the QEMU environment, and the peer is built and run natively on
the host.

Building native client and server
=================================

.. code-block:: console

   $ make -j -C hello_client
   $ make -j -C hello_server

Under ``hello_client``, 4 executables will be generated, and components
used in each layer of them are listed below:

+----------------------+------------+--------------------+------------------+
| hello_client         | TSocket    | TBufferedTransport | TBinaryProtocol  |
+----------------------+------------+--------------------+------------------+
| hello_client_compact | TSocket    | TBufferedTransport | TCompactProtocol |
+----------------------+------------+--------------------+------------------+
| hello_client_zlib    | TSocket    | TZlibTransport     | TBinaryProtocol  |
+----------------------+------------+--------------------+------------------+
| hello_client_ssl     | TSSLSocket | TBufferedTransport | TBinaryProtocol  |
+----------------------+------------+--------------------+------------------+

The same applies for the server. Only the client and the server with the
same set of stacks can communicate.

QEMU network setup
==================

Please refer to the Qemu Setup section in the top-level README.

Running zephyr server in QEMU
=============================

Build the Zephyr version of the thrift application like this:

Build ``echo-server`` sample application like this:

.. zephyr-app-commands::
   :zephyr-app: samples/lib/thrift/hello_server
   :board: <board to use>
   :goals: build
   :compact:


To enable advanced features, extra arguments should be passed accordingly:

- TCompactProtocol: ``-DTHRIFT_COMPACT_PROTOCOL=y``
- TZlibTransport: ``-DTHRIFT_ZLIB_TRANSPORT=y``
- TSSLSocket: ``-DCONF_FILE="prj.conf ../hello_common/overlay-tls.conf"``

Example building for the mps2_an385 with TSSLSocket support:

.. zephyr-app-commands::
   :zephyr-app: samples/lib/thrift/hello_server
   :host-os: unix
   :board: mps2_an385
   :conf: "prj.conf ../hello_common/overlay-tls.conf"
   :goals: run
   :compact:
   

In another terminal, run the ``hello_client`` sample app compiled for the 
host OS:

.. code-block:: console

    $ ./hello_client/hello_client

You should observe the following in the original ``hello_server`` terminal:

.. code-block:: console

    ping
    echo: Hello, world!
    counter: 1
    counter: 2
    counter: 3
    counter: 4
    counter: 5

Running zephyr client in QEMU
=============================

In another terminal, run the ``hello_server`` sample app compiled for the 
host OS:

.. code-block:: console

    $ ./hello_server/hello_server 0.0.0.0


Then, in annother terminal, run the ``hello_client`` sample application 
with:

.. code-block:: console
  
    west build -p auto -b qemu_x86_64 -t run samples/lib/thrift/hello_client \
        -DCONFIG_NET_CONFIG_NEED_IPV6=n \
        -DCONFIG_NET_CONFIG_MY_IPV4_ADDR=\"192.0.2.1\" \
        -DCONFIG_NET_CONFIG_PEER_IPV4_ADDR=\"192.0.2.2\"


The additional arguments for advanced features are the same as 
``hello_client``.

You should observe the following in the original ``hello_client`` terminal:

.. code-block:: console

    Booting from ROM..
    SeaBIOS (version rel-1.15.0-0-g2dd4b9b3f840-prebuilt.qemu.org)
    *** Booting Zephyr OS build zephyr-v3.0.0-1366-gca26ff490759  ***


    [00:00:00.010,000] <inf> net_config: Initializing network
    [00:00:00.010,000] <inf> net_config: IPv4 address: 192.0.2.1
    [00:00:00.110,000] <inf> net_config: IPv6 address: 2001:db8::1
    [00:00:00.110,000] <inf> net_config: IPv6 address: 2001:db8::1
    uart:~$ 
    ping
    echo: Hello, world!
    counter: 1
    counter: 2
    counter: 3
    counter: 4
    counter: 5
    
