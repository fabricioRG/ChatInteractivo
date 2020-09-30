#Chat Interactivo
El chat interactivo es una pequeña aplicación para comunicar dos maquinas por
medio de puertos seriales virtuales, en este caso se utilizaron los puertos
**/dev/ttyUSB0** y **/dev/ttyS1**

##socat
El método de conexión de puertos seriales virtualizados fue la herramienta
socat, se utilizo el siguiente comando para abrir puertos anteriormente descritos
```bash
sudo socat -d -d -d -v -x PTY,link=/dev/ttyUSB0,mode=777,unlink-close,raw,echo=0
PTY,link=/dev/ttyS1,mode=777,unlink-close,raw,echo=0
```

##compilacion
Se utilizo la siguiente instrucción para compilar los archivos y obtener un
ejecutable
```bash
g++ prueba01.cpp -o pro1 -pthread
g++ prueba02.cpp -o pro2 -pthread
```
