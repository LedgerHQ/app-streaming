# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: message-nonano.proto

from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='message-nonano.proto',
  package='',
  syntax='proto3',
  serialized_options=None,
  create_key=_descriptor._internal_create_key,
  serialized_pb=b'\n\x14message-nonano.proto\"\x13\n\x11RequestGetVersion\"%\n\x12ResponseGetVersion\x12\x0f\n\x07version\x18\x01 \x01(\t\"\x11\n\x0fRequestInitSwap\"%\n\x10ResponseInitSwap\x12\x11\n\tdevice_id\x18\x01 \x01(\x0c\"\x11\n\x0fRequestInitSell\"%\n\x10ResponseInitSell\x12\x11\n\tdevice_id\x18\x01 \x01(\x0c\":\n\x07Partner\x12\x0c\n\x04name\x18\x01 \x01(\t\x12\x0e\n\x06pubkey\x18\x02 \x01(\x0c\x12\x11\n\tsignature\x18\x03 \x01(\x0c\"\xb9\x01\n\x0bRequestSwap\x12\x19\n\x07partner\x18\x01 \x01(\x0b\x32\x08.Partner\x12\r\n\x05pb_tx\x18\x02 \x01(\x0c\x12\x11\n\tsignature\x18\x03 \x01(\x0c\x12\x0b\n\x03\x66\x65\x65\x18\x04 \x01(\x0c\x12\x13\n\x0bpayout_path\x18\x05 \x03(\r\x12\x13\n\x0brefund_path\x18\x06 \x03(\r\x12\x1a\n\x12payout_addr_params\x18\x07 \x01(\x0c\x12\x1a\n\x12refund_addr_params\x18\x08 \x01(\x0c\"3\n\x0cResponseSwap\x12\x10\n\x08\x61pproved\x18\x01 \x01(\x08\x12\x11\n\tsignature\x18\x02 \x01(\x0c\"X\n\x0bRequestSell\x12\x19\n\x07partner\x18\x01 \x01(\x0b\x32\x08.Partner\x12\x0e\n\x06\x62\x36\x34_tx\x18\x02 \x01(\x0c\x12\x11\n\tsignature\x18\x03 \x01(\x0c\x12\x0b\n\x03\x66\x65\x65\x18\x04 \x01(\x0c\"3\n\x0cResponseSell\x12\x10\n\x08\x61pproved\x18\x01 \x01(\x08\x12\x11\n\tsignature\x18\x02 \x01(\x0c\"\"\n\rResponseError\x12\x11\n\terror_msg\x18\x01 \x01(\t\"\xcf\x01\n\x07Request\x12)\n\x0bget_version\x18\x01 \x01(\x0b\x32\x12.RequestGetVersionH\x00\x12%\n\tinit_swap\x18\x02 \x01(\x0b\x32\x10.RequestInitSwapH\x00\x12%\n\tinit_sell\x18\x03 \x01(\x0b\x32\x10.RequestInitSellH\x00\x12\x1c\n\x04swap\x18\x04 \x01(\x0b\x32\x0c.RequestSwapH\x00\x12\x1c\n\x04sell\x18\x05 \x01(\x0b\x32\x0c.RequestSellH\x00\x42\x0f\n\rmessage_oneof\"\xf6\x01\n\x08Response\x12*\n\x0bget_version\x18\x01 \x01(\x0b\x32\x13.ResponseGetVersionH\x00\x12&\n\tinit_swap\x18\x02 \x01(\x0b\x32\x11.ResponseInitSwapH\x00\x12&\n\tinit_sell\x18\x03 \x01(\x0b\x32\x11.ResponseInitSellH\x00\x12\x1d\n\x04swap\x18\x04 \x01(\x0b\x32\r.ResponseSwapH\x00\x12\x1d\n\x04sell\x18\x05 \x01(\x0b\x32\r.ResponseSellH\x00\x12\x1f\n\x05\x65rror\x18\x06 \x01(\x0b\x32\x0e.ResponseErrorH\x00\x42\x0f\n\rmessage_oneofb\x06proto3'
)




_REQUESTGETVERSION = _descriptor.Descriptor(
  name='RequestGetVersion',
  full_name='RequestGetVersion',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=24,
  serialized_end=43,
)


_RESPONSEGETVERSION = _descriptor.Descriptor(
  name='ResponseGetVersion',
  full_name='ResponseGetVersion',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
    _descriptor.FieldDescriptor(
      name='version', full_name='ResponseGetVersion.version', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=b"".decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=45,
  serialized_end=82,
)


_REQUESTINITSWAP = _descriptor.Descriptor(
  name='RequestInitSwap',
  full_name='RequestInitSwap',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=84,
  serialized_end=101,
)


_RESPONSEINITSWAP = _descriptor.Descriptor(
  name='ResponseInitSwap',
  full_name='ResponseInitSwap',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
    _descriptor.FieldDescriptor(
      name='device_id', full_name='ResponseInitSwap.device_id', index=0,
      number=1, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=b"",
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=103,
  serialized_end=140,
)


_REQUESTINITSELL = _descriptor.Descriptor(
  name='RequestInitSell',
  full_name='RequestInitSell',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=142,
  serialized_end=159,
)


_RESPONSEINITSELL = _descriptor.Descriptor(
  name='ResponseInitSell',
  full_name='ResponseInitSell',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
    _descriptor.FieldDescriptor(
      name='device_id', full_name='ResponseInitSell.device_id', index=0,
      number=1, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=b"",
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=161,
  serialized_end=198,
)


_PARTNER = _descriptor.Descriptor(
  name='Partner',
  full_name='Partner',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
    _descriptor.FieldDescriptor(
      name='name', full_name='Partner.name', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=b"".decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='pubkey', full_name='Partner.pubkey', index=1,
      number=2, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=b"",
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='signature', full_name='Partner.signature', index=2,
      number=3, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=b"",
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=200,
  serialized_end=258,
)


_REQUESTSWAP = _descriptor.Descriptor(
  name='RequestSwap',
  full_name='RequestSwap',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
    _descriptor.FieldDescriptor(
      name='partner', full_name='RequestSwap.partner', index=0,
      number=1, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='pb_tx', full_name='RequestSwap.pb_tx', index=1,
      number=2, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=b"",
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='signature', full_name='RequestSwap.signature', index=2,
      number=3, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=b"",
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='fee', full_name='RequestSwap.fee', index=3,
      number=4, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=b"",
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='payout_path', full_name='RequestSwap.payout_path', index=4,
      number=5, type=13, cpp_type=3, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='refund_path', full_name='RequestSwap.refund_path', index=5,
      number=6, type=13, cpp_type=3, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='payout_addr_params', full_name='RequestSwap.payout_addr_params', index=6,
      number=7, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=b"",
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='refund_addr_params', full_name='RequestSwap.refund_addr_params', index=7,
      number=8, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=b"",
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=261,
  serialized_end=446,
)


_RESPONSESWAP = _descriptor.Descriptor(
  name='ResponseSwap',
  full_name='ResponseSwap',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
    _descriptor.FieldDescriptor(
      name='approved', full_name='ResponseSwap.approved', index=0,
      number=1, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='signature', full_name='ResponseSwap.signature', index=1,
      number=2, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=b"",
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=448,
  serialized_end=499,
)


_REQUESTSELL = _descriptor.Descriptor(
  name='RequestSell',
  full_name='RequestSell',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
    _descriptor.FieldDescriptor(
      name='partner', full_name='RequestSell.partner', index=0,
      number=1, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='b64_tx', full_name='RequestSell.b64_tx', index=1,
      number=2, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=b"",
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='signature', full_name='RequestSell.signature', index=2,
      number=3, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=b"",
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='fee', full_name='RequestSell.fee', index=3,
      number=4, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=b"",
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=501,
  serialized_end=589,
)


_RESPONSESELL = _descriptor.Descriptor(
  name='ResponseSell',
  full_name='ResponseSell',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
    _descriptor.FieldDescriptor(
      name='approved', full_name='ResponseSell.approved', index=0,
      number=1, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='signature', full_name='ResponseSell.signature', index=1,
      number=2, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=b"",
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=591,
  serialized_end=642,
)


_RESPONSEERROR = _descriptor.Descriptor(
  name='ResponseError',
  full_name='ResponseError',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
    _descriptor.FieldDescriptor(
      name='error_msg', full_name='ResponseError.error_msg', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=b"".decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=644,
  serialized_end=678,
)


_REQUEST = _descriptor.Descriptor(
  name='Request',
  full_name='Request',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
    _descriptor.FieldDescriptor(
      name='get_version', full_name='Request.get_version', index=0,
      number=1, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='init_swap', full_name='Request.init_swap', index=1,
      number=2, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='init_sell', full_name='Request.init_sell', index=2,
      number=3, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='swap', full_name='Request.swap', index=3,
      number=4, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='sell', full_name='Request.sell', index=4,
      number=5, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
    _descriptor.OneofDescriptor(
      name='message_oneof', full_name='Request.message_oneof',
      index=0, containing_type=None,
      create_key=_descriptor._internal_create_key,
    fields=[]),
  ],
  serialized_start=681,
  serialized_end=888,
)


_RESPONSE = _descriptor.Descriptor(
  name='Response',
  full_name='Response',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
    _descriptor.FieldDescriptor(
      name='get_version', full_name='Response.get_version', index=0,
      number=1, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='init_swap', full_name='Response.init_swap', index=1,
      number=2, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='init_sell', full_name='Response.init_sell', index=2,
      number=3, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='swap', full_name='Response.swap', index=3,
      number=4, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='sell', full_name='Response.sell', index=4,
      number=5, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='error', full_name='Response.error', index=5,
      number=6, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
    _descriptor.OneofDescriptor(
      name='message_oneof', full_name='Response.message_oneof',
      index=0, containing_type=None,
      create_key=_descriptor._internal_create_key,
    fields=[]),
  ],
  serialized_start=891,
  serialized_end=1137,
)

_REQUESTSWAP.fields_by_name['partner'].message_type = _PARTNER
_REQUESTSELL.fields_by_name['partner'].message_type = _PARTNER
_REQUEST.fields_by_name['get_version'].message_type = _REQUESTGETVERSION
_REQUEST.fields_by_name['init_swap'].message_type = _REQUESTINITSWAP
_REQUEST.fields_by_name['init_sell'].message_type = _REQUESTINITSELL
_REQUEST.fields_by_name['swap'].message_type = _REQUESTSWAP
_REQUEST.fields_by_name['sell'].message_type = _REQUESTSELL
_REQUEST.oneofs_by_name['message_oneof'].fields.append(
  _REQUEST.fields_by_name['get_version'])
_REQUEST.fields_by_name['get_version'].containing_oneof = _REQUEST.oneofs_by_name['message_oneof']
_REQUEST.oneofs_by_name['message_oneof'].fields.append(
  _REQUEST.fields_by_name['init_swap'])
_REQUEST.fields_by_name['init_swap'].containing_oneof = _REQUEST.oneofs_by_name['message_oneof']
_REQUEST.oneofs_by_name['message_oneof'].fields.append(
  _REQUEST.fields_by_name['init_sell'])
_REQUEST.fields_by_name['init_sell'].containing_oneof = _REQUEST.oneofs_by_name['message_oneof']
_REQUEST.oneofs_by_name['message_oneof'].fields.append(
  _REQUEST.fields_by_name['swap'])
_REQUEST.fields_by_name['swap'].containing_oneof = _REQUEST.oneofs_by_name['message_oneof']
_REQUEST.oneofs_by_name['message_oneof'].fields.append(
  _REQUEST.fields_by_name['sell'])
_REQUEST.fields_by_name['sell'].containing_oneof = _REQUEST.oneofs_by_name['message_oneof']
_RESPONSE.fields_by_name['get_version'].message_type = _RESPONSEGETVERSION
_RESPONSE.fields_by_name['init_swap'].message_type = _RESPONSEINITSWAP
_RESPONSE.fields_by_name['init_sell'].message_type = _RESPONSEINITSELL
_RESPONSE.fields_by_name['swap'].message_type = _RESPONSESWAP
_RESPONSE.fields_by_name['sell'].message_type = _RESPONSESELL
_RESPONSE.fields_by_name['error'].message_type = _RESPONSEERROR
_RESPONSE.oneofs_by_name['message_oneof'].fields.append(
  _RESPONSE.fields_by_name['get_version'])
_RESPONSE.fields_by_name['get_version'].containing_oneof = _RESPONSE.oneofs_by_name['message_oneof']
_RESPONSE.oneofs_by_name['message_oneof'].fields.append(
  _RESPONSE.fields_by_name['init_swap'])
_RESPONSE.fields_by_name['init_swap'].containing_oneof = _RESPONSE.oneofs_by_name['message_oneof']
_RESPONSE.oneofs_by_name['message_oneof'].fields.append(
  _RESPONSE.fields_by_name['init_sell'])
_RESPONSE.fields_by_name['init_sell'].containing_oneof = _RESPONSE.oneofs_by_name['message_oneof']
_RESPONSE.oneofs_by_name['message_oneof'].fields.append(
  _RESPONSE.fields_by_name['swap'])
_RESPONSE.fields_by_name['swap'].containing_oneof = _RESPONSE.oneofs_by_name['message_oneof']
_RESPONSE.oneofs_by_name['message_oneof'].fields.append(
  _RESPONSE.fields_by_name['sell'])
_RESPONSE.fields_by_name['sell'].containing_oneof = _RESPONSE.oneofs_by_name['message_oneof']
_RESPONSE.oneofs_by_name['message_oneof'].fields.append(
  _RESPONSE.fields_by_name['error'])
_RESPONSE.fields_by_name['error'].containing_oneof = _RESPONSE.oneofs_by_name['message_oneof']
DESCRIPTOR.message_types_by_name['RequestGetVersion'] = _REQUESTGETVERSION
DESCRIPTOR.message_types_by_name['ResponseGetVersion'] = _RESPONSEGETVERSION
DESCRIPTOR.message_types_by_name['RequestInitSwap'] = _REQUESTINITSWAP
DESCRIPTOR.message_types_by_name['ResponseInitSwap'] = _RESPONSEINITSWAP
DESCRIPTOR.message_types_by_name['RequestInitSell'] = _REQUESTINITSELL
DESCRIPTOR.message_types_by_name['ResponseInitSell'] = _RESPONSEINITSELL
DESCRIPTOR.message_types_by_name['Partner'] = _PARTNER
DESCRIPTOR.message_types_by_name['RequestSwap'] = _REQUESTSWAP
DESCRIPTOR.message_types_by_name['ResponseSwap'] = _RESPONSESWAP
DESCRIPTOR.message_types_by_name['RequestSell'] = _REQUESTSELL
DESCRIPTOR.message_types_by_name['ResponseSell'] = _RESPONSESELL
DESCRIPTOR.message_types_by_name['ResponseError'] = _RESPONSEERROR
DESCRIPTOR.message_types_by_name['Request'] = _REQUEST
DESCRIPTOR.message_types_by_name['Response'] = _RESPONSE
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

RequestGetVersion = _reflection.GeneratedProtocolMessageType('RequestGetVersion', (_message.Message,), {
  'DESCRIPTOR' : _REQUESTGETVERSION,
  '__module__' : 'message_nonano_pb2'
  # @@protoc_insertion_point(class_scope:RequestGetVersion)
  })
_sym_db.RegisterMessage(RequestGetVersion)

ResponseGetVersion = _reflection.GeneratedProtocolMessageType('ResponseGetVersion', (_message.Message,), {
  'DESCRIPTOR' : _RESPONSEGETVERSION,
  '__module__' : 'message_nonano_pb2'
  # @@protoc_insertion_point(class_scope:ResponseGetVersion)
  })
_sym_db.RegisterMessage(ResponseGetVersion)

RequestInitSwap = _reflection.GeneratedProtocolMessageType('RequestInitSwap', (_message.Message,), {
  'DESCRIPTOR' : _REQUESTINITSWAP,
  '__module__' : 'message_nonano_pb2'
  # @@protoc_insertion_point(class_scope:RequestInitSwap)
  })
_sym_db.RegisterMessage(RequestInitSwap)

ResponseInitSwap = _reflection.GeneratedProtocolMessageType('ResponseInitSwap', (_message.Message,), {
  'DESCRIPTOR' : _RESPONSEINITSWAP,
  '__module__' : 'message_nonano_pb2'
  # @@protoc_insertion_point(class_scope:ResponseInitSwap)
  })
_sym_db.RegisterMessage(ResponseInitSwap)

RequestInitSell = _reflection.GeneratedProtocolMessageType('RequestInitSell', (_message.Message,), {
  'DESCRIPTOR' : _REQUESTINITSELL,
  '__module__' : 'message_nonano_pb2'
  # @@protoc_insertion_point(class_scope:RequestInitSell)
  })
_sym_db.RegisterMessage(RequestInitSell)

ResponseInitSell = _reflection.GeneratedProtocolMessageType('ResponseInitSell', (_message.Message,), {
  'DESCRIPTOR' : _RESPONSEINITSELL,
  '__module__' : 'message_nonano_pb2'
  # @@protoc_insertion_point(class_scope:ResponseInitSell)
  })
_sym_db.RegisterMessage(ResponseInitSell)

Partner = _reflection.GeneratedProtocolMessageType('Partner', (_message.Message,), {
  'DESCRIPTOR' : _PARTNER,
  '__module__' : 'message_nonano_pb2'
  # @@protoc_insertion_point(class_scope:Partner)
  })
_sym_db.RegisterMessage(Partner)

RequestSwap = _reflection.GeneratedProtocolMessageType('RequestSwap', (_message.Message,), {
  'DESCRIPTOR' : _REQUESTSWAP,
  '__module__' : 'message_nonano_pb2'
  # @@protoc_insertion_point(class_scope:RequestSwap)
  })
_sym_db.RegisterMessage(RequestSwap)

ResponseSwap = _reflection.GeneratedProtocolMessageType('ResponseSwap', (_message.Message,), {
  'DESCRIPTOR' : _RESPONSESWAP,
  '__module__' : 'message_nonano_pb2'
  # @@protoc_insertion_point(class_scope:ResponseSwap)
  })
_sym_db.RegisterMessage(ResponseSwap)

RequestSell = _reflection.GeneratedProtocolMessageType('RequestSell', (_message.Message,), {
  'DESCRIPTOR' : _REQUESTSELL,
  '__module__' : 'message_nonano_pb2'
  # @@protoc_insertion_point(class_scope:RequestSell)
  })
_sym_db.RegisterMessage(RequestSell)

ResponseSell = _reflection.GeneratedProtocolMessageType('ResponseSell', (_message.Message,), {
  'DESCRIPTOR' : _RESPONSESELL,
  '__module__' : 'message_nonano_pb2'
  # @@protoc_insertion_point(class_scope:ResponseSell)
  })
_sym_db.RegisterMessage(ResponseSell)

ResponseError = _reflection.GeneratedProtocolMessageType('ResponseError', (_message.Message,), {
  'DESCRIPTOR' : _RESPONSEERROR,
  '__module__' : 'message_nonano_pb2'
  # @@protoc_insertion_point(class_scope:ResponseError)
  })
_sym_db.RegisterMessage(ResponseError)

Request = _reflection.GeneratedProtocolMessageType('Request', (_message.Message,), {
  'DESCRIPTOR' : _REQUEST,
  '__module__' : 'message_nonano_pb2'
  # @@protoc_insertion_point(class_scope:Request)
  })
_sym_db.RegisterMessage(Request)

Response = _reflection.GeneratedProtocolMessageType('Response', (_message.Message,), {
  'DESCRIPTOR' : _RESPONSE,
  '__module__' : 'message_nonano_pb2'
  # @@protoc_insertion_point(class_scope:Response)
  })
_sym_db.RegisterMessage(Response)


# @@protoc_insertion_point(module_scope)