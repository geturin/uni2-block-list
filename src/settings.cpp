#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <algorithm>
#include <stdexcept>
#include "settings.h"
#include "protocol.h"

std::wstring UbWide(const std::string& s) {
    if (s.empty()) return {};
    int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),static_cast<int>(s.size()),nullptr,0);
    if (!n) throw std::runtime_error("无效 UTF-8 内容。");
    std::wstring out(n,L'\0');MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),static_cast<int>(s.size()),out.data(),n);
    return out;
}
std::string UbUtf8(const std::wstring& s) {
    if (s.empty()) return {};
    int n=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,s.data(),static_cast<int>(s.size()),nullptr,0,nullptr,nullptr);
    if (!n) throw std::runtime_error("无效 Unicode 内容。");
    std::string out(n,'\0');WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,s.data(),static_cast<int>(s.size()),out.data(),n,nullptr,nullptr);
    return out;
}
uint64_t UbParseId(const std::wstring& text) {
    uint64_t id=0;
    if (text.empty() || text.size()>20) return 0;
    for (wchar_t c:text) {
        if (c<L'0' || c>L'9' || id>(UINT64_MAX-uint64_t(c-L'0'))/10) return 0;
        id=id*10+uint64_t(c-L'0');
    }
    return UbPlayerId(id)?id:0;
}
namespace {
std::wstring local_folder() {
    wchar_t folder[MAX_PATH]{};
    if (FAILED(SHGetFolderPathW(nullptr,CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE,nullptr,SHGFP_TYPE_CURRENT,folder)))
        throw std::runtime_error("无法取得本机配置目录。");
    return folder;
}
std::wstring settings_folder() {
    const auto dir=local_folder()+L"\\UNI2BlockList";
    if (!CreateDirectoryW(dir.c_str(),nullptr) && GetLastError()!=ERROR_ALREADY_EXISTS)
        throw std::runtime_error("无法创建黑名单配置目录。");
    return dir;
}
// Read the earlier app's files only when no new-format file exists. Leave the
// source untouched, and publish a validated copy without replacing a new file.
std::wstring legacy_path(const wchar_t* name) {
    return local_folder()+L"\\UNI2Blacklist\\"+name;
}
bool read_bytes(const std::wstring& path,std::string& bytes,size_t limit,
    const char* open_error,const char* size_error,const char* read_error) {
    HANDLE file=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    if (file==INVALID_HANDLE_VALUE) {
        const auto code=GetLastError();
        if (code==ERROR_FILE_NOT_FOUND || code==ERROR_PATH_NOT_FOUND) return false;
        throw std::runtime_error(open_error);
    }
    LARGE_INTEGER size{};
    if (!GetFileSizeEx(file,&size) || size.QuadPart<0 || static_cast<uint64_t>(size.QuadPart)>limit) {
        CloseHandle(file);throw std::runtime_error(size_error);
    }
    bytes.assign(static_cast<size_t>(size.QuadPart),'\0');DWORD done=0;
    const bool ok=ReadFile(file,bytes.data(),static_cast<DWORD>(bytes.size()),&done,nullptr) && done==bytes.size();
    CloseHandle(file);
    if (!ok) throw std::runtime_error(read_error);
    return true;
}
bool atomic_write(const std::wstring& target,const std::string& bytes,bool replace,
    const char* open_error,const char* save_error) {
    static volatile LONG sequence=0;
    const auto temp=target+L".tmp."+std::to_wstring(GetCurrentProcessId())+L"."+
        std::to_wstring(static_cast<unsigned long>(InterlockedIncrement(&sequence)));
    HANDLE file=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
    if (file==INVALID_HANDLE_VALUE) throw std::runtime_error(open_error);
    DWORD done=0;
    const bool ok=WriteFile(file,bytes.data(),static_cast<DWORD>(bytes.size()),&done,nullptr) &&
        done==bytes.size() && FlushFileBuffers(file);
    CloseHandle(file);
    if (!ok) {
        DeleteFileW(temp.c_str());throw std::runtime_error(save_error);
    }
    if (MoveFileExW(temp.c_str(),target.c_str(),MOVEFILE_WRITE_THROUGH|(replace?MOVEFILE_REPLACE_EXISTING:0))) return true;
    const auto code=GetLastError();DeleteFileW(temp.c_str());
    if (!replace && (code==ERROR_ALREADY_EXISTS || code==ERROR_FILE_EXISTS)) return false;
    throw std::runtime_error(save_error);
}
std::vector<UbEntry> parse_entries(const std::string& bytes) {
    auto text=UbWide(bytes);
    std::vector<UbEntry> out;size_t start=0;
    while (start<text.size()) {
        size_t end=text.find(L'\n',start);if (end==std::wstring::npos) end=text.size();
        auto line=text.substr(start,end-start);start=end+1;
        if (!line.empty() && line.back()==L'\r') line.pop_back();
        if (line.empty() || line[0]==L'#') continue;
        const auto tab=line.find(L'\t');
        const auto id=UbParseId(line.substr(0,tab));
        if (!id || std::any_of(out.begin(),out.end(),[&](const UbEntry& entry){return entry.id==id;}))
            throw std::runtime_error("黑名单文件包含无效或重复的 SteamID64；原文件保留。");
        const auto alias=tab==std::wstring::npos?L"":line.substr(tab+1);
        if (alias.size()>128 || alias.find_first_of(L"\r\n\t")!=std::wstring::npos || out.size()>=UB_MAX_BLOCKED)
            throw std::runtime_error("黑名单条目超出限制；原文件保留。");
        out.push_back({id,alias});
    }
    return out;
}
std::string serialize_entries(const std::vector<UbEntry>& entries) {
    if (entries.size()>UB_MAX_BLOCKED) throw std::runtime_error("黑名单最多保存 256 人。");
    std::wstring text=L"# UNI2 Block List v1; SteamID64<TAB>alias; UTF-8\n";
    std::vector<uint64_t> seen;
    for (const auto& entry:entries) {
        if (!UbPlayerId(entry.id) || std::find(seen.begin(),seen.end(),entry.id)!=seen.end())
            throw std::runtime_error("SteamID64 无效或重复。");
        seen.push_back(entry.id);
        auto alias=entry.alias.substr(0,128);
        for (auto& character:alias) if (character<L' ' || character==L'\t') character=L' ';
        text+=std::to_wstring(entry.id)+L"\t"+alias+L"\n";
    }
    return UbUtf8(text);
}
bool read_entries(const std::wstring& path,std::vector<UbEntry>& entries) {
    std::string bytes;
    if (!read_bytes(path,bytes,262144,"无法读取黑名单文件。","黑名单文件超过大小限制。","黑名单文件读取不完整。")) return false;
    entries=parse_entries(bytes);return true;
}
std::wstring wifi_path() {
    return settings_folder()+L"\\options.ini";
}
bool read_wifi(const std::wstring& path,bool legacy,bool& blocked) {
    std::string bytes;
    if (!read_bytes(path,bytes,63,"无法读取 Wi-Fi 筛选选项；原文件保留。",
        "Wi-Fi 筛选选项损坏；原文件保留，请修复 options.ini 后重开程序。",
        "Wi-Fi 筛选选项损坏；原文件保留，请修复 options.ini 后重开程序。")) return false;
    const std::string key=legacy?"exclude_wifi=":"block_wifi_players=";
    if (bytes==key+"0\n") {blocked=false;return true;}
    if (bytes==key+"1\n") {blocked=true;return true;}
    throw std::runtime_error("Wi-Fi 筛选选项损坏；原文件保留，请修复 options.ini 后重开程序。");
}
} // namespace

std::wstring UbSettingsPath() {
    return settings_folder()+L"\\block-list.tsv";
}
std::vector<UbEntry> UbLoadSettings() {
    const auto target=UbSettingsPath();std::vector<UbEntry> entries;
    if (read_entries(target,entries)) return entries;
    if (!read_entries(legacy_path(L"blacklist.tsv"),entries)) return {};
    if (atomic_write(target,serialize_entries(entries),false,"无法写入黑名单临时文件。","黑名单保存失败；旧文件保留。")) return entries;
    if (read_entries(target,entries)) return entries;
    throw std::runtime_error("黑名单保存失败；旧文件保留。");
}
void UbSaveSettings(const std::vector<UbEntry>& entries) {
    atomic_write(UbSettingsPath(),serialize_entries(entries),true,"无法写入黑名单临时文件。","黑名单保存失败；旧文件保留。");
}
bool UbLoadWifiOption() {
    const auto target=wifi_path();bool blocked=false;
    if (read_wifi(target,false,blocked)) return blocked;
    if (!read_wifi(legacy_path(L"options.ini"),true,blocked)) return false;
    const std::string bytes=blocked?"block_wifi_players=1\n":"block_wifi_players=0\n";
    if (atomic_write(target,bytes,false,"无法写入 Wi-Fi 筛选选项。","Wi-Fi 筛选选项保存失败；旧文件保留。")) return blocked;
    if (read_wifi(target,false,blocked)) return blocked;
    throw std::runtime_error("Wi-Fi 筛选选项保存失败；旧文件保留。");
}
void UbSaveWifiOption(bool blocked) {
    atomic_write(wifi_path(),blocked?"block_wifi_players=1\n":"block_wifi_players=0\n",true,
        "无法写入 Wi-Fi 筛选选项。","Wi-Fi 筛选选项保存失败；旧文件保留。");
}
