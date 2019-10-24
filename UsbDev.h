#pragma once
#include "BaseType.h"
#include <libusb.h>
namespace NF {
    enum UsbCtrl { 
        UsbCtrl_Open,
        UsbCtrl_Close };
    enum UsbTransfer { 
        UsbTransfer_Send,
        UsbTransfer_Read };
    enum UsbState {
        UsbState_Open,
        UsbState_Close
    };
    enum UsbAttr {
        UsbAttr_Unknow = -1,
        UsbAttr_Ctrl = 0,
        UsbAttr_Sync = 1,
        UsbAttr_Bulk = 2,
        UsbAttr_Interrupt = 3
    };
    struct Endpoint {
        Endpoint(uint8_t _addr, uint8_t _attr) :
            addr(_addr),
            attr((0 <= _attr && _attr <= 3) ? (UsbAttr)_attr : UsbAttr_Unknow)
        {}
        uint8_t addr;
        UsbAttr attr;
    };
    typedef int TransferSize;
    typedef int Result;
    constexpr Timeout TIMEOUT = 0;
    constexpr size_t USB_READ_BUF_SIZE = 64;
    constexpr TransferSize TRANSFER_ERR = -1;
    class UsbDev {
    public:
        UsbDev();
        ~UsbDev();
        bool Open(uint16_t idVendor, uint16_t idProduct);
        void Close();
        TransferSize Send(vector<byte>& data, size_t readBufSize = USB_READ_BUF_SIZE, Timeout time = TIMEOUT);
        TransferSize Read(size_t size = USB_READ_BUF_SIZE, Timeout time = TIMEOUT);
        string_view Name();
        const vector<byte>& GetReadData();
    private:
        Result Transfer(UsbTransfer transfer, vector<byte>& data, TransferSize& actualSize, Timeout time);
        void GetEndpoints();
        UsbState m_state = UsbState_Close;
        libusb_context* m_context = nullptr;
        libusb_device_handle* m_handle = nullptr;
        libusb_device* m_dev = nullptr;
        libusb_device_descriptor m_dscrp{};
        string m_name{};
        vector<Endpoint> m_endpoints{};
        int m_bulkSendEndpoint = -1;
        int m_bulkReadEndpoint = -1;
        vector<byte> m_readData{};
        bool m_readOK = false;
    };
}
