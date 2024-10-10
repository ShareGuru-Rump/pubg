#include <WS2tcpip.h>
#include <iostream>
#include <regex>
#include <unordered_map>
#include <future>
#include <string>
#include <vector>
#include <winhttp.h>
#include <map>
#include <mutex>
#include "nlohmann/json.hpp" 

#pragma comment(lib, "winhttp.lib")


class Rank {
public:
    std::string sendHttpRequestWithWinHttp(const std::wstring& host, const std::wstring& path) {
        HINTERNET hSession = WinHttpOpen(L"WinHTTP Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession) {
            std::wcout << L"WinHttpOpen failed with error code: " << GetLastError() << std::endl;
            return "";
        }

        HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), 8088, 0);
        if (!hConnect) {
            std::wcout << L"WinHttpConnect failed with error code: " << GetLastError() << std::endl;
            WinHttpCloseHandle(hSession);
            return "";
        }

        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
        if (!hRequest) {
            std::wcout << L"WinHttpOpenRequest failed with error code: " << GetLastError() << std::endl;
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return "";
        }

        if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
            std::wcout << L"WinHttpSendRequest failed with error code: " << GetLastError() << std::endl;
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return "";
        }

        if (!WinHttpReceiveResponse(hRequest, NULL)) {
            std::wcout << L"WinHttpReceiveResponse failed with error code: " << GetLastError() << std::endl;
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return "";
        }

        std::string response;
        DWORD bytesRead = 0;
        char buffer[4096];
        while (WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            response.append(buffer, bytesRead);
        }

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        return response;
    }

    std::future<std::string> asyncSendHttpRequest(const std::wstring& host, const std::wstring& path) {
        return std::async(std::launch::async, [this, host, path]() {
            return sendHttpRequestWithWinHttp(host, path);
            });
    }

    void concurrentRequests(const std::vector<std::string>& names, std::unordered_map<std::string, bool>& QuerySuccess, std::unordered_map<std::string, int>& QueryCount) {
        std::map<std::string, std::future<std::string>> futures;
        std::mutex queryMutex;
        std::wstring host = L"pubg1.rump.asia";
        std::wstring seasons = L"31"; // 赛季

        for (const auto& name : names) {
            if (QuerySuccess[name] || QueryCount[name] >= 2) {
                continue;
            }

            std::wstring name_wstr = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(name);
            std::wstring path = L"/api/pubg/getPlayerInfoByName?name=" + name_wstr;

            // 发起异步请求
            futures[name] = asyncSendHttpRequest(host, path);
        }

        for (auto& future : futures) {
        
            try {
                std::string rankResponse = future.second.get(); // 等待请求完成

                PlayerRankList RankList;
                auto jInfo = nlohmann::json::parse(rankResponse);
                int status = jInfo["status"];
                if (status != 200) {
                    std::lock_guard<std::mutex> lock(queryMutex);
                    QueryCount[future.first]++;

                    auto msg = jInfo["msg"];
                    std::cout << "Rank ERROR: " << msg << std::endl;
                    continue;
                }

                // 处理每个模式
                auto solo = jInfo["response"]["solo"];
                auto squad = jInfo["response"]["squad"];
                auto solofpp = jInfo["response"]["solofpp"];
                auto squadfpp = jInfo["response"]["squadfpp"];

                if (solo.is_null())
                {
                    RankList.TPP.Tier = "Unranked";
                    RankList.TPP.SubTier = "";
                    RankList.TPP.RankPoint = 0;
                    RankList.TPP.KDA = 0;
                    RankList.TPP.WinRatio = 0;
                }
                else
                {
                    RankList.TPP.Tier = solo["tier"];
                    RankList.TPP.SubTier = solo["subTier"];
                    RankList.TPP.RankPoint = solo["currentRankPoint"];
                    RankList.TPP.KDA = solo["kda"];
                    RankList.TPP.WinRatio = solo["winRatio"].get<float>() * 100;
                }
                if (solofpp.is_null())
                {
                    RankList.FPP.Tier = "Unranked";
                    RankList.FPP.SubTier = "";
                    RankList.FPP.RankPoint = 0;
                    RankList.FPP.KDA = 0;
                    RankList.FPP.WinRatio = 0;
                }
                else
                {
                    RankList.FPP.Tier = solofpp["tier"];
                    RankList.FPP.SubTier = solofpp["subTier"];
                    RankList.FPP.RankPoint = solofpp["currentRankPoint"];
                    RankList.FPP.KDA = solofpp["kda"];
                    RankList.FPP.WinRatio = solofpp["winRatio"].get<float>() * 100;
                }

                if (squad.is_null())
                {
                    RankList.SquadTPP.Tier = "Unranked";
                    RankList.SquadTPP.SubTier = "";
                    RankList.SquadTPP.RankPoint = 0;
                    RankList.SquadTPP.KDA = 0;
                    RankList.SquadTPP.WinRatio = 0;
                }
                else
                {
                    RankList.SquadTPP.Tier = squad["tier"];
                    RankList.SquadTPP.SubTier = squad["subTier"];
                    RankList.SquadTPP.RankPoint = squad["currentRankPoint"];
                    RankList.SquadTPP.KDA = squad["kda"];
                    RankList.SquadTPP.WinRatio = squad["winRatio"].get<float>() * 100;
                }
                if (squadfpp.is_null())
                {
                    RankList.SquadFPP.Tier = "Unranked";
                    RankList.SquadFPP.SubTier = "";
                    RankList.SquadFPP.RankPoint = 0;
                    RankList.SquadFPP.KDA = 0;
                    RankList.SquadFPP.WinRatio = 0;
                }
                else
                {
                    RankList.SquadFPP.Tier = squadfpp["tier"];
                    RankList.SquadFPP.SubTier = squadfpp["subTier"];
                    RankList.SquadFPP.RankPoint = squadfpp["currentRankPoint"];
                    RankList.SquadFPP.KDA = squadfpp["kda"];
                    RankList.SquadFPP.WinRatio = squadfpp["winRatio"].get<float>() * 100;
                }

                Data::SetPlayerRankListsItem(future.first, RankList);
                {
                    std::lock_guard<std::mutex> lock(queryMutex);
                    QuerySuccess[future.first] = true;
                }
                std::cout << "Rank Success: " << future.first << std::endl;
            }
            catch (const std::exception&) {
                std::lock_guard<std::mutex> lock(queryMutex);
                QueryCount[future.first]++;
                std::cerr << "02 ERROR:" << std::endl;
            }
         
        }
    }

    static void Update() {
        Rank rank;
        std::unordered_map<std::string, PlayerRankList> PlayerRankLists;

        std::vector<std::string> pPlayers;

        std::unordered_map<std::string, bool> QuerySuccess;
        std::unordered_map<std::string, int> QueryCount;

        while (true) {
            PlayerRankLists = Data::GetPlayerRankLists();

            if (!GameData.Config.ESP.Tier && !GameData.Config.ESP.RankPoint && !GameData.Config.ESP.KDA) {
                PlayerRankLists.clear();
                pPlayers.clear();
                QuerySuccess.clear();
                QueryCount.clear();
                Sleep(GameData.ThreadSleep);
                continue;
            }

            for (auto& player : PlayerRankLists) {
                if (QuerySuccess.find(player.first) == QuerySuccess.end()) {
                    QuerySuccess[player.first] = false;
                }

                if (QueryCount.find(player.first) == QueryCount.end()) {
                    QueryCount[player.first] = 0;
                }

                if (QuerySuccess[player.first]) {
                    continue;
                }

                pPlayers.push_back(player.first);
            }

            if (!pPlayers.empty()) {
                rank.concurrentRequests(pPlayers, QuerySuccess, QueryCount);
            }

            PlayerRankLists.clear();
            pPlayers.clear();
        }
    }
};

