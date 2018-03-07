#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>

/*
struct Direccion { Port, IP  }
//Listener(50000)
//Cuando se conecta alguien guardo su puerto e IP bajo un vector de struct (Direccion)
for(int i = 0; i < numPlayerMinimo; i++) {
TcpSocket sock
sock.accept
if(vector.lenght > 0) { //Hay alguien esperando ya y le mandamos con un packet el número de jugadores esperando + IP y Puerto de cada usuario en espera encadenados (for que viaja por todo el vector)}
else { //No hay nadie más. Mandarle un mensaje con un 0 como número de jugadores }
vector.push(sock.getRemoteAdress, sock.getRemotePort) //En el cliente se mirará si el número de jugadores es > 0.
sock.disconnect()
}
listener.close()
*/

#define MIN_PLAYERS 4

struct Direction
{
	std::string ip;
	int port;
};

std::vector<Direction> awaitingPlayers;

void main() {
	awaitingPlayers = std::vector<Direction>();

	sf::TcpSocket socket;
	sf::TcpListener listener;
	listener.listen(50000);

	for (int i = 0; i < MIN_PLAYERS; i++) {
		sf::Socket::Status st = listener.accept(socket);
		if (st == sf::Socket::Status::Done) {
			//Miro cuántos jugadores hay esperando.
				//Si hay 0 => Le digo que no hay nadie: packet << 0
				//Si hay > 0 => Le paso un paquete con: packet << numPlayersWaiting << Ip << Port << Ip << Port << ...

			//Guardo puerto e IP de este cliente para los siguientes.
			sf::Packet pak;
			int playerWaiting = awaitingPlayers.size();
			std::cout << "Num players waiting: " << playerWaiting << std::endl;
			pak << playerWaiting;
			if (playerWaiting > 0) {
				for (int j = 0; j < playerWaiting; j++) {
					pak << awaitingPlayers[j].ip << awaitingPlayers[j].port;
					std::cout << "Player #" << j << " IP: " << awaitingPlayers[j].ip << " PORT: " << awaitingPlayers[j].port << std::endl;
				}
			}
			socket.send(pak);
			Direction temp;
			temp.ip = socket.getRemoteAddress().toString();
			temp.port = socket.getRemotePort();
			awaitingPlayers.push_back(temp);
			std::cout << "Num players waiting after conecting player #" << i << ": " << awaitingPlayers.size() << std::endl;
		}
		socket.disconnect();
	}

	listener.close();
}