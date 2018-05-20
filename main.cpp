#include <iostream>
#include "Client.h"

int main(int argc, char *argv[]) {
  try {
    if (argc < 2)
      throw std::runtime_error(
          "Please input URL code in arguments of command line");

    Client client(argv[1]);
    if (client.Connect()) client.LoadData();

  } catch (std::exception &err) {
    std::cout << err.what() << std::endl;
  }
  return 0;
}