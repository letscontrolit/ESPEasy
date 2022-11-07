# SPI_RECEIVE

Connect TX(A-CH) and RX(B-CH) with a short wire   
Connect TX(B-CH) and RX(A-CH) with a short wire   

__Connections should be as short as possible__

![Arduino](https://user-images.githubusercontent.com/6020549/71318829-8bca1000-24d9-11ea-829b-4a07f90ac1e9.jpg)

# Channel baudrate
You can specify different baudrates for channel A and channel B

# Sketch of the other side
```
#define baudrate 9600L

void setup() {
  Serial.begin(baudrate);
}

void loop() {
  char buf[64];
  sprintf(buf,"Hello Wold, Baudrate is %ld", baudrate);
  Serial.println(buf);
  //Serial.println("Hello this is Arduino");
  delay(1000);
}
```

