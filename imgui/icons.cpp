#include "pch.h"
#include "icons.h"

static const unsigned int settings_icon_compressed_size = 304;
static const unsigned int settings_icon_compressed_data[304 / 4] = {
    0x0000bc57, 0x00000000, 0x10050000, 0x00000400, 0x008e0020, 0xffffff23, 0x20038209, 0x20038220, 0xa81a8e12, 0x203b820e, 0x20038225, 0x830382ee,
    0x82fd2002, 0x82bf2004, 0xa8312003, 0x82288a40, 0x82472037, 0x86f72003, 0x2006834b, 0x200482f9, 0x98228a44, 0x2027820a, 0x921c8b26, 0x82e8204b,
    0x00082113, 0x1f82009a, 0x03821b20, 0x0382f220, 0x4b8e3e20, 0x938a7f20, 0x54200a83, 0x0b8f1f8b, 0x3a202082, 0xf4202f86, 0x42200782, 0x8f921f83,
    0xff211282, 0x971b8378, 0x20208203, 0x20778a27, 0x830b82f6, 0x0b1f4187, 0x69201e87, 0x17874397, 0xd6202882, 0x0e93c38e, 0x2f876220, 0x34820797,
    0x03824120, 0x3c93fc20, 0xf820138a, 0x4c200b82, 0x938a4397, 0x27825120, 0x4bc6ef20, 0x47820b20, 0x23415920, 0x82652006, 0x926e2007, 0x8797a4a7,
    0x20a78a9f, 0xfa4b405c, 0x07411c20, 0x2207a707, 0x41ffffff, 0xb7200f2f, 0x02211382, 0x8a00b200, 0x4343834b, 0x00201cbf, 0x4b82009e, 0xffff212c,
    0x000004ff, 0x00000000, 0xfa050000, 0x4ff2775d,
};


const unsigned int reload_icon_compressed_size = 179;
const unsigned int reload_icon_compressed_data[180 / 4] = {
    0x0000bc57, 0x00000000, 0x00040000, 0x00000400, 0x00da0020, 0xfefefe24, 0x008afff5, 0x328a6aa7, 0x479b0a94, 0x2b87308f, 0x4b201782, 0xaf828b9f,
    0x2387fb20, 0x078f3787, 0x3b9f439b, 0x2a878b8a, 0x7382a620, 0x0c82df20, 0x0382bf20, 0x5b8aaf20, 0x0b825f20, 0x2f874f20, 0x6b8b0793, 0x0f83f38f,
    0xea205782, 0x838f3793, 0x3f930f83, 0x039b1783, 0x1741eb8f, 0x9247931b, 0x8f65203f, 0x8e379313, 0x83ac20af, 0x8303870f, 0x970b873f, 0x8fa7a707,
    0x200f828b, 0x203baa3b, 0x13534102, 0xa397538f, 0xfe231787, 0xa7f8fefe, 0xaf338773, 0x33fa0507, 0x00bc5866,
};

static const unsigned int injector_icon_compressed_size = 204;
static const unsigned int injector_icon_compressed_data[204 / 4] =
{
    0x0000bc57, 0x00000000, 0x44080000, 0x00000400, 0x00ae0020, 0x0086ff20, 0x1393368b, 0x0ba31f8b, 0xa3234383, 0x83ff5f52, 0x830b8303, 0x23078333,
    0xff333645, 0x13870383, 0x03b31783, 0x8bbb5f40, 0x071f41bb, 0x41071341, 0x578f3b1f, 0x41177f41, 0x57972f7b, 0xcb415f93, 0x8f579f2b, 0xa75ba75f,
    0xa75faf57, 0x073f4357, 0x17435fa7, 0x9333870b, 0x0bb74107, 0xcb418b87, 0x42578b1b, 0x579f0757, 0xaf076341, 0x9b5f9357, 0x8b678743, 0x87af9307,
    0xd35f8b57, 0x37934457, 0xafafb3a3, 0x9f071341, 0x8357b75b, 0x405b9b5f, 0x6343a357, 0x3fab4607, 0xbb0f0741, 0x8e0020fb, 0x4bef4600, 0x00205a8e,
    0x1b8fff8b, 0xfa050fbf, 0x702be02e,
};

unsigned int stb_decompress_length(const unsigned char* input);
unsigned int stb_decompress(unsigned char* output, const unsigned char* i, unsigned int length);

BOOL CreateIcon(LPDIRECT3DDEVICE9 pDevice, LPDIRECT3DTEXTURE9* pTexture, const unsigned char* pData, unsigned int w, unsigned int h) {
    if (pDevice->CreateTexture(w, h, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, pTexture, NULL) != D3D_OK)
        return FALSE;

    D3DLOCKED_RECT tex_locked_rect;
    if ((*pTexture)->LockRect(0, &tex_locked_rect, NULL, 0) != D3D_OK) {
        (*pTexture)->Release();
        return FALSE;
    }

    int row_size = w * 4;
    for (unsigned int y = 0; y < h; y++)
        memcpy((PBYTE)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, pData + row_size * y, row_size);

    (*pTexture)->UnlockRect(0);
    return TRUE;
}

BOOL CreateIconCompressed(LPDIRECT3DDEVICE9 pDevice, LPDIRECT3DTEXTURE9* pTexture, const unsigned int* data, const unsigned int size, unsigned int w, unsigned int h) {
    const unsigned int buf_decompressed_size = stb_decompress_length((const unsigned char*)data);
    BYTE* buf_decompressed_data = new BYTE[buf_decompressed_size];
    stb_decompress(buf_decompressed_data, (const unsigned char*)data, size);

    BOOL ret = CreateIcon(pDevice, pTexture, buf_decompressed_data, w, h);

    delete[] buf_decompressed_data;
    return ret;
}