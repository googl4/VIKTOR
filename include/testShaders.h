const uint32_t vertShaderData[] = {
	0x07230203,0x00010000,0x00080008,0x0000002d,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x000b000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000d,0x00000019,0x00000024,
	0x00000025,0x00000029,0x0000002b,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,
	0x6e69616d,0x00000000,0x00060005,0x0000000b,0x505f6c67,0x65567265,0x78657472,0x00000000,
	0x00060006,0x0000000b,0x00000000,0x505f6c67,0x7469736f,0x006e6f69,0x00070006,0x0000000b,
	0x00000001,0x505f6c67,0x746e696f,0x657a6953,0x00000000,0x00070006,0x0000000b,0x00000002,
	0x435f6c67,0x4470696c,0x61747369,0x0065636e,0x00070006,0x0000000b,0x00000003,0x435f6c67,
	0x446c6c75,0x61747369,0x0065636e,0x00030005,0x0000000d,0x00000000,0x00050005,0x00000011,
	0x736e6f63,0x746e6174,0x00000073,0x00040006,0x00000011,0x00000000,0x0070766d,0x00030005,
	0x00000013,0x00000000,0x00030005,0x00000019,0x00736f70,0x00040005,0x00000024,0x726f4e76,
	0x006c616d,0x00040005,0x00000025,0x6d726f6e,0x00006c61,0x00030005,0x00000029,0x00565576,
	0x00030005,0x0000002b,0x00007675,0x00050048,0x0000000b,0x00000000,0x0000000b,0x00000000,
	0x00050048,0x0000000b,0x00000001,0x0000000b,0x00000001,0x00050048,0x0000000b,0x00000002,
	0x0000000b,0x00000003,0x00050048,0x0000000b,0x00000003,0x0000000b,0x00000004,0x00030047,
	0x0000000b,0x00000002,0x00040048,0x00000011,0x00000000,0x00000005,0x00050048,0x00000011,
	0x00000000,0x00000023,0x00000000,0x00050048,0x00000011,0x00000000,0x00000007,0x00000010,
	0x00030047,0x00000011,0x00000002,0x00040047,0x00000019,0x0000001e,0x00000000,0x00040047,
	0x00000024,0x0000001e,0x00000000,0x00040047,0x00000025,0x0000001e,0x00000001,0x00040047,
	0x00000029,0x0000001e,0x00000001,0x00040047,0x0000002b,0x0000001e,0x00000002,0x00020013,
	0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,0x00000020,0x00040017,
	0x00000007,0x00000006,0x00000004,0x00040015,0x00000008,0x00000020,0x00000000,0x0004002b,
	0x00000008,0x00000009,0x00000001,0x0004001c,0x0000000a,0x00000006,0x00000009,0x0006001e,
	0x0000000b,0x00000007,0x00000006,0x0000000a,0x0000000a,0x00040020,0x0000000c,0x00000003,
	0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000003,0x00040015,0x0000000e,0x00000020,
	0x00000001,0x0004002b,0x0000000e,0x0000000f,0x00000000,0x00040018,0x00000010,0x00000007,
	0x00000004,0x0003001e,0x00000011,0x00000010,0x00040020,0x00000012,0x00000009,0x00000011,
	0x0004003b,0x00000012,0x00000013,0x00000009,0x00040020,0x00000014,0x00000009,0x00000010,
	0x00040017,0x00000017,0x00000006,0x00000003,0x00040020,0x00000018,0x00000001,0x00000017,
	0x0004003b,0x00000018,0x00000019,0x00000001,0x0004002b,0x00000006,0x0000001b,0x3f800000,
	0x00040020,0x00000021,0x00000003,0x00000007,0x00040020,0x00000023,0x00000003,0x00000017,
	0x0004003b,0x00000023,0x00000024,0x00000003,0x0004003b,0x00000018,0x00000025,0x00000001,
	0x00040017,0x00000027,0x00000006,0x00000002,0x00040020,0x00000028,0x00000003,0x00000027,
	0x0004003b,0x00000028,0x00000029,0x00000003,0x00040020,0x0000002a,0x00000001,0x00000027,
	0x0004003b,0x0000002a,0x0000002b,0x00000001,0x00050036,0x00000002,0x00000004,0x00000000,
	0x00000003,0x000200f8,0x00000005,0x00050041,0x00000014,0x00000015,0x00000013,0x0000000f,
	0x0004003d,0x00000010,0x00000016,0x00000015,0x0004003d,0x00000017,0x0000001a,0x00000019,
	0x00050051,0x00000006,0x0000001c,0x0000001a,0x00000000,0x00050051,0x00000006,0x0000001d,
	0x0000001a,0x00000001,0x00050051,0x00000006,0x0000001e,0x0000001a,0x00000002,0x00070050,
	0x00000007,0x0000001f,0x0000001c,0x0000001d,0x0000001e,0x0000001b,0x00050091,0x00000007,
	0x00000020,0x00000016,0x0000001f,0x00050041,0x00000021,0x00000022,0x0000000d,0x0000000f,
	0x0003003e,0x00000022,0x00000020,0x0004003d,0x00000017,0x00000026,0x00000025,0x0003003e,
	0x00000024,0x00000026,0x0004003d,0x00000027,0x0000002c,0x0000002b,0x0003003e,0x00000029,
	0x0000002c,0x000100fd,0x00010038
};

const uint32_t fragShaderData[] = {
	0x07230203,0x00010000,0x00080008,0x00000029,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x0008000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x00000023,0x00000028,
	0x00030010,0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,
	0x6e69616d,0x00000000,0x00040005,0x00000009,0x6f6c6f63,0x00007275,0x00030005,0x0000000f,
	0x00786574,0x00050005,0x00000010,0x736e6f63,0x746e6174,0x00000073,0x00060006,0x00000010,
	0x00000000,0x6867696c,0x72694474,0x00000000,0x00050006,0x00000010,0x00000001,0x78644974,
	0x00000000,0x00030005,0x00000012,0x00000000,0x00030005,0x0000001d,0x00000073,0x00030005,
	0x00000023,0x00007675,0x00040005,0x00000028,0x6d726f6e,0x00006c61,0x00040047,0x00000009,
	0x0000001e,0x00000000,0x00040047,0x0000000f,0x00000022,0x00000000,0x00040047,0x0000000f,
	0x00000021,0x00000000,0x00050048,0x00000010,0x00000000,0x00000023,0x00000040,0x00050048,
	0x00000010,0x00000001,0x00000023,0x00000050,0x00030047,0x00000010,0x00000002,0x00040047,
	0x0000001d,0x00000022,0x00000000,0x00040047,0x0000001d,0x00000021,0x00000001,0x00040047,
	0x00000023,0x0000001e,0x00000001,0x00040047,0x00000028,0x0000001e,0x00000000,0x00020013,
	0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,0x00000020,0x00040017,
	0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,0x00000007,0x0004003b,
	0x00000008,0x00000009,0x00000003,0x00090019,0x0000000a,0x00000006,0x00000001,0x00000000,
	0x00000000,0x00000000,0x00000001,0x00000000,0x00040015,0x0000000b,0x00000020,0x00000000,
	0x0004002b,0x0000000b,0x0000000c,0x00000040,0x0004001c,0x0000000d,0x0000000a,0x0000000c,
	0x00040020,0x0000000e,0x00000000,0x0000000d,0x0004003b,0x0000000e,0x0000000f,0x00000000,
	0x0004001e,0x00000010,0x00000007,0x0000000b,0x00040020,0x00000011,0x00000009,0x00000010,
	0x0004003b,0x00000011,0x00000012,0x00000009,0x00040015,0x00000013,0x00000020,0x00000001,
	0x0004002b,0x00000013,0x00000014,0x00000001,0x00040020,0x00000015,0x00000009,0x0000000b,
	0x00040020,0x00000018,0x00000000,0x0000000a,0x0002001a,0x0000001b,0x00040020,0x0000001c,
	0x00000000,0x0000001b,0x0004003b,0x0000001c,0x0000001d,0x00000000,0x0003001b,0x0000001f,
	0x0000000a,0x00040017,0x00000021,0x00000006,0x00000002,0x00040020,0x00000022,0x00000001,
	0x00000021,0x0004003b,0x00000022,0x00000023,0x00000001,0x00040017,0x00000026,0x00000006,
	0x00000003,0x00040020,0x00000027,0x00000001,0x00000026,0x0004003b,0x00000027,0x00000028,
	0x00000001,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,0x00000005,
	0x00050041,0x00000015,0x00000016,0x00000012,0x00000014,0x0004003d,0x0000000b,0x00000017,
	0x00000016,0x00050041,0x00000018,0x00000019,0x0000000f,0x00000017,0x0004003d,0x0000000a,
	0x0000001a,0x00000019,0x0004003d,0x0000001b,0x0000001e,0x0000001d,0x00050056,0x0000001f,
	0x00000020,0x0000001a,0x0000001e,0x0004003d,0x00000021,0x00000024,0x00000023,0x00050057,
	0x00000007,0x00000025,0x00000020,0x00000024,0x0003003e,0x00000009,0x00000025,0x000100fd,
	0x00010038
};

const size_t vertShaderSize = sizeof( vertShaderData );
const size_t fragShaderSize = sizeof( fragShaderData );

const uint32_t vertShaderData2[] = {
	0x07230203,0x00010000,0x00080008,0x00000033,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x0008000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000001f,0x00000023,0x0000002f,
	0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,0x00000000,0x00050005,
	0x0000000c,0x69736f70,0x6e6f6974,0x00000073,0x00030005,0x00000013,0x00737675,0x00060005,
	0x0000001d,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x0000001d,0x00000000,
	0x505f6c67,0x7469736f,0x006e6f69,0x00070006,0x0000001d,0x00000001,0x505f6c67,0x746e696f,
	0x657a6953,0x00000000,0x00070006,0x0000001d,0x00000002,0x435f6c67,0x4470696c,0x61747369,
	0x0065636e,0x00070006,0x0000001d,0x00000003,0x435f6c67,0x446c6c75,0x61747369,0x0065636e,
	0x00030005,0x0000001f,0x00000000,0x00060005,0x00000023,0x565f6c67,0x65747265,0x646e4978,
	0x00007865,0x00030005,0x0000002f,0x00007675,0x00050048,0x0000001d,0x00000000,0x0000000b,
	0x00000000,0x00050048,0x0000001d,0x00000001,0x0000000b,0x00000001,0x00050048,0x0000001d,
	0x00000002,0x0000000b,0x00000003,0x00050048,0x0000001d,0x00000003,0x0000000b,0x00000004,
	0x00030047,0x0000001d,0x00000002,0x00040047,0x00000023,0x0000000b,0x0000002a,0x00040047,
	0x0000002f,0x0000001e,0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,
	0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000002,0x00040015,
	0x00000008,0x00000020,0x00000000,0x0004002b,0x00000008,0x00000009,0x00000003,0x0004001c,
	0x0000000a,0x00000007,0x00000009,0x00040020,0x0000000b,0x00000006,0x0000000a,0x0004003b,
	0x0000000b,0x0000000c,0x00000006,0x0004002b,0x00000006,0x0000000d,0xbf800000,0x0005002c,
	0x00000007,0x0000000e,0x0000000d,0x0000000d,0x0004002b,0x00000006,0x0000000f,0x40400000,
	0x0005002c,0x00000007,0x00000010,0x0000000d,0x0000000f,0x0005002c,0x00000007,0x00000011,
	0x0000000f,0x0000000d,0x0006002c,0x0000000a,0x00000012,0x0000000e,0x00000010,0x00000011,
	0x0004003b,0x0000000b,0x00000013,0x00000006,0x0004002b,0x00000006,0x00000014,0x00000000,
	0x0005002c,0x00000007,0x00000015,0x00000014,0x00000014,0x0004002b,0x00000006,0x00000016,
	0x40000000,0x0005002c,0x00000007,0x00000017,0x00000014,0x00000016,0x0005002c,0x00000007,
	0x00000018,0x00000016,0x00000014,0x0006002c,0x0000000a,0x00000019,0x00000015,0x00000017,
	0x00000018,0x00040017,0x0000001a,0x00000006,0x00000004,0x0004002b,0x00000008,0x0000001b,
	0x00000001,0x0004001c,0x0000001c,0x00000006,0x0000001b,0x0006001e,0x0000001d,0x0000001a,
	0x00000006,0x0000001c,0x0000001c,0x00040020,0x0000001e,0x00000003,0x0000001d,0x0004003b,
	0x0000001e,0x0000001f,0x00000003,0x00040015,0x00000020,0x00000020,0x00000001,0x0004002b,
	0x00000020,0x00000021,0x00000000,0x00040020,0x00000022,0x00000001,0x00000020,0x0004003b,
	0x00000022,0x00000023,0x00000001,0x00040020,0x00000025,0x00000006,0x00000007,0x0004002b,
	0x00000006,0x00000028,0x3f800000,0x00040020,0x0000002c,0x00000003,0x0000001a,0x00040020,
	0x0000002e,0x00000003,0x00000007,0x0004003b,0x0000002e,0x0000002f,0x00000003,0x00050036,
	0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,0x00000005,0x0003003e,0x0000000c,
	0x00000012,0x0003003e,0x00000013,0x00000019,0x0004003d,0x00000020,0x00000024,0x00000023,
	0x00050041,0x00000025,0x00000026,0x0000000c,0x00000024,0x0004003d,0x00000007,0x00000027,
	0x00000026,0x00050051,0x00000006,0x00000029,0x00000027,0x00000000,0x00050051,0x00000006,
	0x0000002a,0x00000027,0x00000001,0x00070050,0x0000001a,0x0000002b,0x00000029,0x0000002a,
	0x00000014,0x00000028,0x00050041,0x0000002c,0x0000002d,0x0000001f,0x00000021,0x0003003e,
	0x0000002d,0x0000002b,0x0004003d,0x00000020,0x00000030,0x00000023,0x00050041,0x00000025,
	0x00000031,0x00000013,0x00000030,0x0004003d,0x00000007,0x00000032,0x00000031,0x0003003e,
	0x0000002f,0x00000032,0x000100fd,0x00010038
};

const uint32_t fragShaderData2[] = {
	0x07230203,0x00010000,0x00080008,0x0000003d,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000020,0x00000036,0x00030010,
	0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
	0x00000000,0x00060005,0x0000000a,0x656e696c,0x65447261,0x28687470,0x003b3166,0x00030005,
	0x00000009,0x0000007a,0x00030005,0x00000013,0x0000007a,0x00050005,0x00000016,0x74706564,
	0x78655468,0x00000000,0x00030005,0x0000001a,0x00000073,0x00030005,0x00000020,0x00007675,
	0x00030005,0x00000027,0x00000064,0x00040005,0x00000028,0x61726170,0x0000006d,0x00030005,
	0x0000002d,0x00000063,0x00030005,0x0000002e,0x00786574,0x00040005,0x00000036,0x6f6c6f63,
	0x00007275,0x00040047,0x00000016,0x00000022,0x00000000,0x00040047,0x00000016,0x00000021,
	0x00000001,0x00040047,0x0000001a,0x00000022,0x00000000,0x00040047,0x0000001a,0x00000021,
	0x00000002,0x00040047,0x00000020,0x0000001e,0x00000000,0x00040047,0x0000002e,0x00000022,
	0x00000000,0x00040047,0x0000002e,0x00000021,0x00000000,0x00040047,0x00000036,0x0000001e,
	0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
	0x00000020,0x00040020,0x00000007,0x00000007,0x00000006,0x00040021,0x00000008,0x00000006,
	0x00000007,0x0004002b,0x00000006,0x0000000c,0x3dcccccd,0x0004002b,0x00000006,0x0000000f,
	0x44fa0000,0x00090019,0x00000014,0x00000006,0x00000001,0x00000000,0x00000000,0x00000000,
	0x00000001,0x00000000,0x00040020,0x00000015,0x00000000,0x00000014,0x0004003b,0x00000015,
	0x00000016,0x00000000,0x0002001a,0x00000018,0x00040020,0x00000019,0x00000000,0x00000018,
	0x0004003b,0x00000019,0x0000001a,0x00000000,0x0003001b,0x0000001c,0x00000014,0x00040017,
	0x0000001e,0x00000006,0x00000002,0x00040020,0x0000001f,0x00000001,0x0000001e,0x0004003b,
	0x0000001f,0x00000020,0x00000001,0x00040017,0x00000022,0x00000006,0x00000004,0x00040015,
	0x00000024,0x00000020,0x00000000,0x0004002b,0x00000024,0x00000025,0x00000000,0x00040017,
	0x0000002b,0x00000006,0x00000003,0x00040020,0x0000002c,0x00000007,0x0000002b,0x0004003b,
	0x00000015,0x0000002e,0x00000000,0x00040020,0x00000035,0x00000003,0x00000022,0x0004003b,
	0x00000035,0x00000036,0x00000003,0x0004002b,0x00000006,0x00000038,0x00000000,0x00050036,
	0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,0x00000005,0x0004003b,0x00000007,
	0x00000013,0x00000007,0x0004003b,0x00000007,0x00000027,0x00000007,0x0004003b,0x00000007,
	0x00000028,0x00000007,0x0004003b,0x0000002c,0x0000002d,0x00000007,0x0004003d,0x00000014,
	0x00000017,0x00000016,0x0004003d,0x00000018,0x0000001b,0x0000001a,0x00050056,0x0000001c,
	0x0000001d,0x00000017,0x0000001b,0x0004003d,0x0000001e,0x00000021,0x00000020,0x00050057,
	0x00000022,0x00000023,0x0000001d,0x00000021,0x00050051,0x00000006,0x00000026,0x00000023,
	0x00000000,0x0003003e,0x00000013,0x00000026,0x0004003d,0x00000006,0x00000029,0x00000013,
	0x0003003e,0x00000028,0x00000029,0x00050039,0x00000006,0x0000002a,0x0000000a,0x00000028,
	0x0003003e,0x00000027,0x0000002a,0x0004003d,0x00000014,0x0000002f,0x0000002e,0x0004003d,
	0x00000018,0x00000030,0x0000001a,0x00050056,0x0000001c,0x00000031,0x0000002f,0x00000030,
	0x0004003d,0x0000001e,0x00000032,0x00000020,0x00050057,0x00000022,0x00000033,0x00000031,
	0x00000032,0x0008004f,0x0000002b,0x00000034,0x00000033,0x00000033,0x00000000,0x00000001,
	0x00000002,0x0003003e,0x0000002d,0x00000034,0x0004003d,0x0000002b,0x00000037,0x0000002d,
	0x00050051,0x00000006,0x00000039,0x00000037,0x00000000,0x00050051,0x00000006,0x0000003a,
	0x00000037,0x00000001,0x00050051,0x00000006,0x0000003b,0x00000037,0x00000002,0x00070050,
	0x00000022,0x0000003c,0x00000039,0x0000003a,0x0000003b,0x00000038,0x0003003e,0x00000036,
	0x0000003c,0x000100fd,0x00010038,0x00050036,0x00000006,0x0000000a,0x00000000,0x00000008,
	0x00030037,0x00000007,0x00000009,0x000200f8,0x0000000b,0x0004003d,0x00000006,0x0000000d,
	0x00000009,0x00050088,0x00000006,0x0000000e,0x0000000c,0x0000000d,0x00050088,0x00000006,
	0x00000010,0x0000000e,0x0000000f,0x000200fe,0x00000010,0x00010038
};

const size_t vertShaderSize2 = sizeof( vertShaderData2 );
const size_t fragShaderSize2 = sizeof( fragShaderData2 );
