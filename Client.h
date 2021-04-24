#pragma once
#pragma comment(lib, "ws2_32.lib")
#include <functional>
#include <iostream>
#include <map>
#include <vector>
#include <WinSock2.h>
#include "Error.h"
#include <thread>
#include <Server.h>

#pragma warning( push )
#pragma warning( disable: 4996 )

class Client
{
private:
	const u_short hostshort_;
	const char* inet_addr_;

	typedef std::function<void(const char*, SOCKET connection)> on_function;

	std::map<const char*, std::function<void(const char*, SOCKET connection)>> on_functions_;
	std::function<void(const char*, SOCKET connection)> on_any_function_ = [](const char* ch, SOCKET socket) { };

	SOCKET connection {};

public:
	Client(const u_short hostshort = 7070, const char* inet_address = "127.0.0.1") :
		hostshort_(hostshort),
		inet_addr_(inet_address)
	{

	}

	void start()
	{
		WSAData wsa_data;
		const WORD dll_version = MAKEWORD(2, 1);
		if (WSAStartup(dll_version, &wsa_data) != 0)
		{
			vl::err<WORD>("WORD & WSAData");
		}

		SOCKADDR_IN sockaddr_in;
		sockaddr_in.sin_addr.S_un.S_addr = inet_addr(inet_addr_);
		sockaddr_in.sin_port = htons(hostshort_);
		sockaddr_in.sin_family = AF_INET;

		connection = socket(AF_INET, SOCK_STREAM, NULL);

		if (connect(connection, (SOCKADDR*)&sockaddr_in, sizeof(sockaddr_in)) != 0)
		{
			vl::err<WORD>("connection");
		}

		std::thread* thread = new std::thread
		(
			[&]()
			{
				int size_of_packet = 0;
				while (true)
				{
					if (recv(connection, (char*)&size_of_packet, sizeof(int), NULL) != -1)
					{
						char* packet = new char[size_of_packet + 1];
						if (recv(connection, packet, size_of_packet, NULL) != -1)
						{
							packet[size_of_packet] = '\0';
							for (const auto& it : on_functions_)
							{
								if (strcmp(it.first, packet) == 0)
								{
									it.second(packet, connection);
									break;
								}
							}

							on_any_function_(packet, connection);
						}
						delete[] packet;
					}
				}
			}
		);

		on
		(
			NULL, 
			[&](const char* packet, SOCKET)
			{
				
			}
		);

	}

	bool emit(const char* packet)
	{
		int sizeof_packet = strlen(packet);
		send(connection, (char*)&sizeof_packet, sizeof(int), NULL);
		send(connection, packet, sizeof_packet, NULL);
		return true;
	}

	void emit(const char* packet, DWORD delay)
	{
		emit(packet);
		Sleep(delay);
	}

	void on(const char* packet, const std::function<void(const char*, SOCKET socket)>& on_function)
	{
		std::cout << packet << std::endl;
		on_functions_[packet] = on_function;
	}

	void onAny(const std::function<void(const char*, SOCKET socket)>& on_any_function)
	{
		on_any_function_= on_any_function;
	}

};

#pragma warning( pop )
