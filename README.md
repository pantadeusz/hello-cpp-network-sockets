# hello-cpp-network-sockets
As simple as possible example of C++17 sockets handling using async

The goal of this example is to provide quick start for people who would like to start programming sockets in new projects.

I tried to make it simple, but with advanced techniques from C++17 like initialization of variables along ```if``` statement, async calls with futures and ```auto``` types.

## Compilation

```bash
g++ client.cpp -std=c++17 -o client
g++ server.cpp -std=c++17 -o server
```

## Running

```bash
./server &
./client
```

## Credits

Tadeusz Puźniakowski


## Contribution

(TODO) - suggestions?
