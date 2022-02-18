import logging
import sys

# apt install python3-protobuf
import message_pb2

logger = logging.getLogger("plugin")

class Plugin:
    def __init__(self):
        logger.debug("plugin loaded")
        self.state = 1

    def recv(self, data):
        self.state += 1

    def send(self):
        if self.state == 0:
            return b'\n\x00'
        elif self.state == 1:
            return b'\x1a\x1f\n\x1dThisIsAComplicatedTransaction'
        else:
            assert False

class PluginX:
    def __init__(self):
        logger.debug("plugin loaded")
        self.state = 0

    def recv(self, data):
        response = message_pb2.Response()
        response.ParseFromString(data)
        
        if self.state == 0:
            assert response.WhichOneof("message_oneof") == "get_version"
            print(f"version: {response.get_version.version}")
        elif self.state == 1:
            assert response.WhichOneof("message_oneof") == "sign_tx"
            print(f"version: {response.sign_tx.signature.hex()}")

        self.state += 1

    def send(self):
        #return b'\n\x00'

        message = message_pb2.Message()
        if self.state == 0:
            get_version = message_pb2.MessageGetVersion()
            message.get_version.CopyFrom(get_version)
            assert message.WhichOneof("message_oneof") == "get_version"
        elif self.state == 1:
            sign_tx = message_pb2.MessageSignTx()
            sign_tx.tx = b"ThisIsAComplicatedTransaction"
            message.sign_tx.CopyFrom(sign_tx)
            assert message.WhichOneof("message_oneof") == "sign_tx"
        else:
            assert False

        return message.SerializeToString()


if __name__ == "__main__":
    plugin = PluginX()
    data = plugin.send()
    print(data)
    plugin.recv(b'\n\x10\n\x0eversion 1.33.7')

    data = plugin.send()
    print(data)
    plugin.recv(b'\x1a"\n \xdaXF\xf6\xeeT_\xad\x9b\xe5\xf7\x98\x14;*\x92\x98\xae*v\x03f\xd2\xb4z\x86\xeb\xe7\xb2\xab\xfc`')
    
    response = message_pb2.Response()
    response.ParseFromString(b'\n\x10\n\x0eversion 1.33.7') 

    assert response.WhichOneof("message_oneof") == "get_version"
    print(f"version: {response.get_version.version}")


    response = message_pb2.Response()
    response.ParseFromString(b'\x1a"\n \xdaXF\xf6\xeeT_\xad\x9b\xe5\xf7\x98\x14;*\x92\x98\xae*v\x03f\xd2\xb4z\x86\xeb\xe7\xb2\xab\xfc`')

    assert response.WhichOneof("message_oneof") == "sign_tx"
    print(f"version: {response.sign_tx.signature.hex()}")
