#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <cstring>
#include <iostream>
#include <mutex>
#include <thread>
#include <time.h>


#define MAX_MENSAJES 25

std::vector<std::string> aMensajes;
bool connected = false;
std::thread receiveThread;
sf::IpAddress ip = sf::IpAddress::getLocalAddress();
sf::TcpSocket socket;
char connectionType, mode;
char buffer[2000];
std::size_t received;
std::string text = "Connected to: ";
int ticks = 0;
std::string windowName;
sf::Socket::Status st;
sf::Color color;
std::string mensaje;
std::vector<sf::TcpSocket*> peers;

//Hacer que cliente y servidor tengan mensajes de diferente color
struct Message {
	std::string s;
	sf::Color textColor;
};
struct Direction {
	std::string ip;
	int port;
};


void addMessage(std::string s) {
	aMensajes.push_back(s);
	if (aMensajes.size() > 25) {
		aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
	}
}

void nonBlockingSend(std::string _sentMessage) {
	std::size_t bytesSent;
	std::string subString;
	for (int i = 0; i < peers.size(); i++) {
		st = peers[i]->send((" >" + _sentMessage).c_str(), (" >" + _sentMessage).length() + 1, bytesSent);
		subString = (" >" + _sentMessage);
		if (st == sf::Socket::Status::Partial) {
			bool completed = false;
			while (!completed) {
				if (bytesSent < subString.length()) {
					//No todo se ha enviado
					subString = subString.substr(bytesSent + 1, subString.length() - bytesSent);
					st = peers[i]->send(subString.c_str(), subString.length() + 1, bytesSent);
				}
				else { completed = true; }
			}
		}
	}

	addMessage(_sentMessage);
}

void nonBlockedComunication() {
	bool endedGaem = false;

	//SCREEN SETUP
	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), windowName);

	sf::Font font;
	if (!font.loadFromFile("courbd.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

	mensaje = "";

	sf::Text chattingText(mensaje, font, 14);

	chattingText.setFillColor(color);
	chattingText.setStyle(sf::Text::Regular);


	sf::Text text(mensaje, font, 14);
	text.setFillColor(color);
	text.setStyle(sf::Text::Regular);
	text.setPosition(0, 560);

	sf::RectangleShape separator(sf::Vector2f(800, 5));
	separator.setFillColor(sf::Color(200, 200, 200, 255));
	separator.setPosition(0, 550);

	while (!endedGaem && window.isOpen()/*Game not ended*/) {

		//Receive:
		//Si no recibo nada paso == NOT READY
		//Si recibo algo lo proceso == DONE
		std::vector<int> pos;
		bool deletePeer = false;
		for (int i = 0; i < peers.size(); i++) {

			std::size_t receivedData;
			char data[2000];
			st = peers[i]->receive(data, 2000, receivedData);
			if (st == sf::Socket::Status::Done) {
				//PROCESAR INFORMACION
				std::string receivedMessage = data;
				receivedMessage = receivedMessage.substr(0, receivedData);
				if (strcmp(receivedMessage.c_str(), " >exit") == 0) {
					std::cout << "EXIT" << std::endl;
					std::string exitMessage = " >exit";
					//nonBlockingSend(exitMessage);
					addMessage(" >DISONNECTED FROM CHAT");
				}
				else { addMessage(receivedMessage); }

			}
			else if (st == sf::Socket::Status::Disconnected) {
				deletePeer = true;
				pos.push_back(i);
			}
		}
		if (deletePeer) {
			for (int i = pos.size() - 1; i >= 0; i--) {
				peers.erase(peers.begin() + pos[i]);
			}
		}


		//Eventos SFML
		//Close/Disconnect
		//close app == break
		//Send
		//Si escribo algo lo mando y proceso el Partial OTRO WHILE

		//Pintar

		sf::Event evento;
		while (window.pollEvent(evento))
		{
			std::string exitMessage;
			switch (evento.type)
			{
			case sf::Event::Closed:
				//DISCONECT FROM SERVER
				endedGaem = true;
				std::cout << "CLOSE" << std::endl;
				connected = false;
				exitMessage = " >exit";
				nonBlockingSend(exitMessage);
				window.close();
				break;
			case sf::Event::KeyPressed:
				if (evento.key.code == sf::Keyboard::Escape)
					window.close();
				else if (evento.key.code == sf::Keyboard::Return) //envia mensaje
				{
					nonBlockingSend(mensaje);
					if (strcmp(mensaje.c_str(), "exit") == 0) {
						std::cout << "EXIT" << std::endl;
						addMessage("YOU DISCONNECTED FROM CHAT");
						connected = false;
						endedGaem = true;
						window.close();
					}
					mensaje = "";
				}
				break;
			case sf::Event::TextEntered:
				if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
					mensaje += (char)evento.text.unicode;
				else if (evento.text.unicode == 8 && mensaje.length() > 0)
					mensaje.erase(mensaje.length() - 1, mensaje.length());
				break;
			}
		}

		window.draw(separator);
		for (size_t i = 0; i < aMensajes.size(); i++)
		{
			std::string chatting = aMensajes[i];
			chattingText.setPosition(sf::Vector2f(0, 20 * i));
			chattingText.setString(chatting);
			window.draw(chattingText);
		}
		std::string mensaje_ = mensaje + "_";
		text.setString(mensaje_);
		window.draw(text);


		window.display();
		window.clear();
	}

}

int main()
{
	srand(time(NULL));
	color = sf::Color(rand() % 255 + 0, rand() % 255 + 0, rand() % 255 + 0, 255);
	peers = std::vector<sf::TcpSocket*>();
	bool serv;
	std::string startPeer;

	std::cout << "Enter (p) to peer or anything else to not to peer" << std::endl;
	std::cin >> startPeer;

	if (startPeer == "p") {
		std::cout << "P pressed" << std::endl;
		//conectarse al bootstrap server
		do {
			ticks++;
			st = socket.connect(ip, 50000, sf::seconds(5.f));
			if (st != sf::Socket::Status::Done) std::cout << "NO SE PUDO CONECTAR TRAS 5s" << std::endl;
		} while (st != sf::Socket::Status::Done && ticks < 2);

		sf::Packet packet;

		st = socket.receive(packet); //recibir packet de jugadores conectados con todo ok
		
		if (st == sf::Socket::Status::Done) {
			int numplayers;
			//recibir direcciones sockets y conectarse
			packet >> numplayers;
			std::cout << "Numero de jugadores: " << numplayers << std::endl;
			if (numplayers > 0) {
				for (int i = 0; i < numplayers; i++) {
					Direction tempDir;
					sf::TcpSocket* tempSock = new sf::TcpSocket;
					packet >> tempDir.ip;
					packet >> tempDir.port;
					std::cout << "Player #" << i << " IP: " << tempDir.ip << " PORT: " << tempDir.port << std::endl;

					sf::Socket::Status st2 = tempSock->connect(tempDir.ip, tempDir.port, sf::seconds(5.f));
					std::cout << st2 << std::endl;
					peers.push_back(tempSock);
				}
			}
			std::cout << "LOCAL PORT: " << socket.getLocalPort() << std::endl;
			int tempPort = socket.getLocalPort();
			socket.disconnect();
			//esperar al resto de jugadores
			while (peers.size() < 3) {
				std::cout << "Esperando a un nuevo peer" << std::endl;
				sf::TcpListener listener;
				sf::TcpSocket* tempSock = new sf::TcpSocket;
				std::cout << "LOCAL PORT: " << tempPort << std::endl;
				listener.listen(tempPort);
				listener.accept(*tempSock);
				peers.push_back(tempSock);
				std::cout << "Se ha conectado un nuevo peer " << tempSock->getRemotePort() << std::endl;
			}
			std::cout << "numero de jugadores en peers: " << peers.size() << std::endl;
			//poner nonblocking a los peers
			for (int i = 0; i < peers.size(); i++) {
				peers[i]->setBlocking(false);
			}
			//empezar
			std::cout << "Iniciando nonBlockedCommunication" << std::endl;
			nonBlockedComunication();
			

		}
	}
}