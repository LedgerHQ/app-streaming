# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: message.proto

import sys
_b=sys.version_info[0]<3 and (lambda x:x) or (lambda x:x.encode('latin1'))
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='message.proto',
  package='',
  syntax='proto3',
  serialized_pb=_b('\n\rmessage.proto\"\x13\n\x11RequestGetVersion\"%\n\x12ResponseGetVersion\x12\x0f\n\x07version\x18\x01 \x01(\t\"I\n\x10RequestGetPubKey\x12\x0c\n\x04path\x18\x01 \x03(\r\x12\x0f\n\x07\x63onfirm\x18\x02 \x01(\x08\x12\x16\n\x0eget_chain_code\x18\x03 \x01(\x08\"Z\n\x11ResponseGetPubKey\x12\x10\n\x08\x61pproved\x18\x01 \x01(\x08\x12\x0e\n\x06pubkey\x18\x02 \x01(\x0c\x12\x0f\n\x07\x61\x64\x64ress\x18\x03 \x01(\x0c\x12\x12\n\nchain_code\x18\x04 \x01(\x0c\"\x1b\n\rRequestSignTx\x12\n\n\x02tx\x18\x01 \x01(\x0c\"5\n\x0eResponseSignTx\x12\x10\n\x08\x61pproved\x18\x01 \x01(\x08\x12\x11\n\tsignature\x18\x02 \x01(\x0c\"\"\n\rResponseError\x12\x11\n\terror_msg\x18\x01 \x01(\t\"\x91\x01\n\x07Request\x12)\n\x0bget_version\x18\x01 \x01(\x0b\x32\x12.RequestGetVersionH\x00\x12\'\n\nget_pubkey\x18\x02 \x01(\x0b\x32\x11.RequestGetPubKeyH\x00\x12!\n\x07sign_tx\x18\x03 \x01(\x0b\x32\x0e.RequestSignTxH\x00\x42\x0f\n\rmessage_oneof\"\xb6\x01\n\x08Response\x12*\n\x0bget_version\x18\x01 \x01(\x0b\x32\x13.ResponseGetVersionH\x00\x12(\n\nget_pubkey\x18\x02 \x01(\x0b\x32\x12.ResponseGetPubKeyH\x00\x12\"\n\x07sign_tx\x18\x03 \x01(\x0b\x32\x0f.ResponseSignTxH\x00\x12\x1f\n\x05\x65rror\x18\x04 \x01(\x0b\x32\x0e.ResponseErrorH\x00\x42\x0f\n\rmessage_oneofb\x06proto3')
)
_sym_db.RegisterFileDescriptor(DESCRIPTOR)




_REQUESTGETVERSION = _descriptor.Descriptor(
  name='RequestGetVersion',
  full_name='RequestGetVersion',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=17,
  serialized_end=36,
)


_RESPONSEGETVERSION = _descriptor.Descriptor(
  name='ResponseGetVersion',
  full_name='ResponseGetVersion',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='version', full_name='ResponseGetVersion.version', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=38,
  serialized_end=75,
)


_REQUESTGETPUBKEY = _descriptor.Descriptor(
  name='RequestGetPubKey',
  full_name='RequestGetPubKey',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='path', full_name='RequestGetPubKey.path', index=0,
      number=1, type=13, cpp_type=3, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='confirm', full_name='RequestGetPubKey.confirm', index=1,
      number=2, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='get_chain_code', full_name='RequestGetPubKey.get_chain_code', index=2,
      number=3, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=77,
  serialized_end=150,
)


_RESPONSEGETPUBKEY = _descriptor.Descriptor(
  name='ResponseGetPubKey',
  full_name='ResponseGetPubKey',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='approved', full_name='ResponseGetPubKey.approved', index=0,
      number=1, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='pubkey', full_name='ResponseGetPubKey.pubkey', index=1,
      number=2, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='address', full_name='ResponseGetPubKey.address', index=2,
      number=3, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='chain_code', full_name='ResponseGetPubKey.chain_code', index=3,
      number=4, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=152,
  serialized_end=242,
)


_REQUESTSIGNTX = _descriptor.Descriptor(
  name='RequestSignTx',
  full_name='RequestSignTx',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='tx', full_name='RequestSignTx.tx', index=0,
      number=1, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=244,
  serialized_end=271,
)


_RESPONSESIGNTX = _descriptor.Descriptor(
  name='ResponseSignTx',
  full_name='ResponseSignTx',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='approved', full_name='ResponseSignTx.approved', index=0,
      number=1, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='signature', full_name='ResponseSignTx.signature', index=1,
      number=2, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=273,
  serialized_end=326,
)


_RESPONSEERROR = _descriptor.Descriptor(
  name='ResponseError',
  full_name='ResponseError',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='error_msg', full_name='ResponseError.error_msg', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=328,
  serialized_end=362,
)


_REQUEST = _descriptor.Descriptor(
  name='Request',
  full_name='Request',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='get_version', full_name='Request.get_version', index=0,
      number=1, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='get_pubkey', full_name='Request.get_pubkey', index=1,
      number=2, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='sign_tx', full_name='Request.sign_tx', index=2,
      number=3, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
    _descriptor.OneofDescriptor(
      name='message_oneof', full_name='Request.message_oneof',
      index=0, containing_type=None, fields=[]),
  ],
  serialized_start=365,
  serialized_end=510,
)


_RESPONSE = _descriptor.Descriptor(
  name='Response',
  full_name='Response',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='get_version', full_name='Response.get_version', index=0,
      number=1, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='get_pubkey', full_name='Response.get_pubkey', index=1,
      number=2, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='sign_tx', full_name='Response.sign_tx', index=2,
      number=3, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='error', full_name='Response.error', index=3,
      number=4, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
    _descriptor.OneofDescriptor(
      name='message_oneof', full_name='Response.message_oneof',
      index=0, containing_type=None, fields=[]),
  ],
  serialized_start=513,
  serialized_end=695,
)

_REQUEST.fields_by_name['get_version'].message_type = _REQUESTGETVERSION
_REQUEST.fields_by_name['get_pubkey'].message_type = _REQUESTGETPUBKEY
_REQUEST.fields_by_name['sign_tx'].message_type = _REQUESTSIGNTX
_REQUEST.oneofs_by_name['message_oneof'].fields.append(
  _REQUEST.fields_by_name['get_version'])
_REQUEST.fields_by_name['get_version'].containing_oneof = _REQUEST.oneofs_by_name['message_oneof']
_REQUEST.oneofs_by_name['message_oneof'].fields.append(
  _REQUEST.fields_by_name['get_pubkey'])
_REQUEST.fields_by_name['get_pubkey'].containing_oneof = _REQUEST.oneofs_by_name['message_oneof']
_REQUEST.oneofs_by_name['message_oneof'].fields.append(
  _REQUEST.fields_by_name['sign_tx'])
_REQUEST.fields_by_name['sign_tx'].containing_oneof = _REQUEST.oneofs_by_name['message_oneof']
_RESPONSE.fields_by_name['get_version'].message_type = _RESPONSEGETVERSION
_RESPONSE.fields_by_name['get_pubkey'].message_type = _RESPONSEGETPUBKEY
_RESPONSE.fields_by_name['sign_tx'].message_type = _RESPONSESIGNTX
_RESPONSE.fields_by_name['error'].message_type = _RESPONSEERROR
_RESPONSE.oneofs_by_name['message_oneof'].fields.append(
  _RESPONSE.fields_by_name['get_version'])
_RESPONSE.fields_by_name['get_version'].containing_oneof = _RESPONSE.oneofs_by_name['message_oneof']
_RESPONSE.oneofs_by_name['message_oneof'].fields.append(
  _RESPONSE.fields_by_name['get_pubkey'])
_RESPONSE.fields_by_name['get_pubkey'].containing_oneof = _RESPONSE.oneofs_by_name['message_oneof']
_RESPONSE.oneofs_by_name['message_oneof'].fields.append(
  _RESPONSE.fields_by_name['sign_tx'])
_RESPONSE.fields_by_name['sign_tx'].containing_oneof = _RESPONSE.oneofs_by_name['message_oneof']
_RESPONSE.oneofs_by_name['message_oneof'].fields.append(
  _RESPONSE.fields_by_name['error'])
_RESPONSE.fields_by_name['error'].containing_oneof = _RESPONSE.oneofs_by_name['message_oneof']
DESCRIPTOR.message_types_by_name['RequestGetVersion'] = _REQUESTGETVERSION
DESCRIPTOR.message_types_by_name['ResponseGetVersion'] = _RESPONSEGETVERSION
DESCRIPTOR.message_types_by_name['RequestGetPubKey'] = _REQUESTGETPUBKEY
DESCRIPTOR.message_types_by_name['ResponseGetPubKey'] = _RESPONSEGETPUBKEY
DESCRIPTOR.message_types_by_name['RequestSignTx'] = _REQUESTSIGNTX
DESCRIPTOR.message_types_by_name['ResponseSignTx'] = _RESPONSESIGNTX
DESCRIPTOR.message_types_by_name['ResponseError'] = _RESPONSEERROR
DESCRIPTOR.message_types_by_name['Request'] = _REQUEST
DESCRIPTOR.message_types_by_name['Response'] = _RESPONSE

RequestGetVersion = _reflection.GeneratedProtocolMessageType('RequestGetVersion', (_message.Message,), dict(
  DESCRIPTOR = _REQUESTGETVERSION,
  __module__ = 'message_pb2'
  # @@protoc_insertion_point(class_scope:RequestGetVersion)
  ))
_sym_db.RegisterMessage(RequestGetVersion)

ResponseGetVersion = _reflection.GeneratedProtocolMessageType('ResponseGetVersion', (_message.Message,), dict(
  DESCRIPTOR = _RESPONSEGETVERSION,
  __module__ = 'message_pb2'
  # @@protoc_insertion_point(class_scope:ResponseGetVersion)
  ))
_sym_db.RegisterMessage(ResponseGetVersion)

RequestGetPubKey = _reflection.GeneratedProtocolMessageType('RequestGetPubKey', (_message.Message,), dict(
  DESCRIPTOR = _REQUESTGETPUBKEY,
  __module__ = 'message_pb2'
  # @@protoc_insertion_point(class_scope:RequestGetPubKey)
  ))
_sym_db.RegisterMessage(RequestGetPubKey)

ResponseGetPubKey = _reflection.GeneratedProtocolMessageType('ResponseGetPubKey', (_message.Message,), dict(
  DESCRIPTOR = _RESPONSEGETPUBKEY,
  __module__ = 'message_pb2'
  # @@protoc_insertion_point(class_scope:ResponseGetPubKey)
  ))
_sym_db.RegisterMessage(ResponseGetPubKey)

RequestSignTx = _reflection.GeneratedProtocolMessageType('RequestSignTx', (_message.Message,), dict(
  DESCRIPTOR = _REQUESTSIGNTX,
  __module__ = 'message_pb2'
  # @@protoc_insertion_point(class_scope:RequestSignTx)
  ))
_sym_db.RegisterMessage(RequestSignTx)

ResponseSignTx = _reflection.GeneratedProtocolMessageType('ResponseSignTx', (_message.Message,), dict(
  DESCRIPTOR = _RESPONSESIGNTX,
  __module__ = 'message_pb2'
  # @@protoc_insertion_point(class_scope:ResponseSignTx)
  ))
_sym_db.RegisterMessage(ResponseSignTx)

ResponseError = _reflection.GeneratedProtocolMessageType('ResponseError', (_message.Message,), dict(
  DESCRIPTOR = _RESPONSEERROR,
  __module__ = 'message_pb2'
  # @@protoc_insertion_point(class_scope:ResponseError)
  ))
_sym_db.RegisterMessage(ResponseError)

Request = _reflection.GeneratedProtocolMessageType('Request', (_message.Message,), dict(
  DESCRIPTOR = _REQUEST,
  __module__ = 'message_pb2'
  # @@protoc_insertion_point(class_scope:Request)
  ))
_sym_db.RegisterMessage(Request)

Response = _reflection.GeneratedProtocolMessageType('Response', (_message.Message,), dict(
  DESCRIPTOR = _RESPONSE,
  __module__ = 'message_pb2'
  # @@protoc_insertion_point(class_scope:Response)
  ))
_sym_db.RegisterMessage(Response)


# @@protoc_insertion_point(module_scope)