#include "UsbDev.h"
#include "Console.h"
namespace NF {
    UsbDev::UsbDev() {}
    UsbDev::~UsbDev() {
        Close();
    }
    bool UsbDev::Open(uint16_t idVendor, uint16_t idProduct) {
        Result r = libusb_init(&m_context);
        auto& console = Console::Instance();
        if (r < 0) {
            console.Add(u8"初始化失败: " + to_string(r), Log_Error);
            return false;
        }
        libusb_device** list = nullptr;
        ssize_t size = libusb_get_device_list(nullptr, &list);
        if (size < 0) {
            console.Add(u8"获取设备失败: size = " + to_string(size), Log_Error);
            return false;
        }
        for (ssize_t i = 0; list[i] != nullptr; ++i) {
            libusb_device* dev = list[i];
            libusb_device_descriptor dscrp{};
            r = libusb_get_device_descriptor(dev, &dscrp);
            if (r < 0) {
                console.Add(u8"获取设备信息失败: " + to_string(r), Log_Error);
                return false;
            }
            if (dscrp.idVendor == idVendor &&
                dscrp.idProduct == idProduct) {
                r = libusb_open(dev, &m_handle);
                if (r < 0) {
                    console.Add(u8"设备打开失败: " + to_string(r) + u8"，请重新安装驱动", Log_Error);
                    return false;
                }
                console.Add(u8"设备打开成功");
                m_dev = dev;
                m_dscrp = dscrp;
                break;
            }
        }
        if (m_handle) {
            r = libusb_set_configuration(m_handle, 1);
        } else {
            console.Add(u8"设备打开失败: 请检查是否已经连接设备", Log_Error);
            return false;
        }
        if (r < 0) {
            console.Add(u8"设置设备失败: " + to_string(r), Log_Error);
            return false;
        }
        r = libusb_claim_interface(m_handle, 0);
        if (r < 0) {
            console.Add(u8"声明接口失败: " + to_string(r), Log_Error);
            return false;
        }
        m_state = UsbState_Open;
        return true;
    }
    void UsbDev::Close() {
        if (m_handle) {
            libusb_release_interface(m_handle, 0);
            libusb_close(m_handle);
            m_handle = nullptr;
        }
        if (m_context) { 
            libusb_exit(m_context);
            m_context = nullptr;
        }
        Console::Instance().Add(u8"设备已关闭");
        m_state = UsbState_Close;
    }
    TransferSize UsbDev::Send(vector<byte>& data, size_t readBufSize, Timeout time) {
        auto& console = Console::Instance();
        console.Add(u8"数据发送中...", Log_Debug);
        TransferSize actualSize = 0;
        Result r = Transfer(UsbTransfer_Send, data, actualSize, time);
        if (r < 0) {
            console.Add(u8"数据发送失败: " + to_string(r), Log_Error);
            console.Add(libusb_error_name(r), Log_Error);
            return TRANSFER_ERR;
        }
        auto PrintSendLog = [&]() {
            stringstream ss{};
            ss << u8"发送字节 (" << actualSize << "/" << data.size() << "): ";
            for (const auto& d : data) {
                ss << std::setw(2) << std::setfill('0') << std::hex << (unsigned int)d << " ";
            }
            console.Add(u8"数据发送成功", Log_Debug);
            console.Add(ss.str(), Log_Debug);
        };
        PrintSendLog();
        Read(readBufSize, time);
        return actualSize;
    }
    TransferSize UsbDev::Read(size_t size, Timeout time) {
        auto& console = Console::Instance();
        console.Add(u8"数据读取中...", Log_Debug);
        TransferSize actualSize = 0;
        m_readData.clear();
        m_readData.resize(size);
        int i = 0;
        while (i < 2) {
            Result r = Transfer(UsbTransfer_Read, m_readData, actualSize, time);
            if (r >= 0) {
                auto PrintReadLog = [&]() {
                    stringstream ss{};
                    ss << u8"读取字节 (" << actualSize << "/" << m_readData.size() << "): ";
                    m_readData.resize(actualSize);
                    for (const auto& d : m_readData) {
                        ss << std::setw(2) << std::setfill('0') << std::hex << (unsigned int)d << " ";
                    }
                    console.Add(u8"数据读取成功", Log_Debug);
                    console.Add(ss.str(), Log_Debug);
                };
                PrintReadLog();
                m_readOK = true;
                return actualSize;
            }
            console.Add(u8"数据读取失败" + to_string(r), Log_Error);
            console.Add(libusb_error_name(r), Log_Error);
            ++i;
        }
        m_readOK = false;
        return TRANSFER_ERR;
    }
    const vector<byte>& UsbDev::GetReadData() {
        return m_readData;
    }
    Result UsbDev::Transfer(
        UsbTransfer transfer,
        vector<byte>& data,
        TransferSize& actualSize,
        Timeout time) {
        auto GetI = [&]() {
            return transfer == UsbTransfer_Send ?
                m_bulkSendEndpoint : m_bulkReadEndpoint;
        };
        int i = GetI();
        if (i == -1) {
            GetEndpoints();
            i = GetI();
            if (i == -1) {
                Console::Instance().Add(u8"传输数据失败", Log_Error);
                return -1;
            }
        }
        int r = libusb_bulk_transfer(
            m_handle,
            m_endpoints[i].addr,
            data.data(),
            (TransferSize)data.size(),
            &actualSize,
            time);
        return r;
    }
    string_view UsbDev::Name() {
        m_name.clear();
        if (m_state != UsbState_Open) {
            return m_name;
        }
        auto GetStr = [&](uint8_t data) -> string {
            char str[256];
            stringstream ss{};
            if (data && libusb_get_string_descriptor_ascii(m_handle, data, 
                (byte*)str, sizeof(str)) > 0) {
                ss << str;
            } else {
                ss.width(4);
                ss.fill('0');
                ss << std::hex << data;
            }
            return ss.str();
        };
        m_name += GetStr(m_dscrp.iManufacturer) + " - " + 
            GetStr(m_dscrp.iProduct);
        return m_name;
    }
    void UsbDev::GetEndpoints() {
        m_bulkSendEndpoint = -1;
        m_bulkReadEndpoint = -1;
        m_endpoints.clear();
        if (m_state != UsbState_Open) {
            return;
        }
        Result r = 0;
        libusb_config_descriptor* cfg;
        for (int i = 0; i < m_dscrp.bNumConfigurations; ++i) {
            r = libusb_get_config_descriptor(m_dev, (uint8_t)i, &cfg);
            if (r < 0) {
                Console::Instance().Add(u8"获取设置信息失败: " + to_string(r), Log_Error);
                continue;
            }
            for (int j = 0; j < cfg->bNumInterfaces; ++j) {
                auto interface = &cfg->interface[j];
                for (int k = 0; k < interface->num_altsetting; ++k) {
                    auto altsetting = &interface->altsetting[k];
                    for (int l = 0; l < altsetting->bNumEndpoints; ++l) {
                        auto endpoint = &altsetting->endpoint[l];
                        Endpoint ep{ 
                            endpoint->bEndpointAddress,
                            endpoint->bmAttributes };
                        m_endpoints.push_back(ep);
                        if (ep.attr == UsbAttr_Bulk) {
                            if (ep.addr < 0x80) {
                                m_bulkSendEndpoint = (int)m_endpoints.size() - 1;
                            } else {
                                m_bulkReadEndpoint = (int)m_endpoints.size() - 1;
                            }
                        }
                    }
                }
            }
            libusb_free_config_descriptor(cfg);
        }
    }
}
