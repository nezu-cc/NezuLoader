#pragma once

BOOL CreateIconCompressed(LPDIRECT3DDEVICE9 pDevice, LPDIRECT3DTEXTURE9* pTexture, const unsigned int* data, const unsigned int size, unsigned int w, unsigned int h);

#define DEFINE_CPMPRESSED_ICON(name, data_name, w, h)\
    extern const unsigned int data_name ## _compressed_size;\
    extern const unsigned int data_name ## _compressed_data[];\
    inline BOOL Create ## name ## Icon(LPDIRECT3DDEVICE9 pDevice, LPDIRECT3DTEXTURE9* pTexture) {\
        return CreateIconCompressed(pDevice, pTexture, data_name ## _compressed_data, data_name ## _compressed_size, w, h);\
    }

DEFINE_CPMPRESSED_ICON(Settings, settings_icon, 18, 18);
DEFINE_CPMPRESSED_ICON(Reload, reload_icon, 16, 16);
DEFINE_CPMPRESSED_ICON(Injector, injector_icon, 23, 23);