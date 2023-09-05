# LAB 2: Camada de Aplicação

Esta pasta compreende o segundo laboratório da matéria CES35.

## Execução

Para executar e testar o código, devem ser abertos dois terminais. No primeiro, iremos iniciar o `server`.
```
cd Server
sh server.sh
```
No segundo, iremos iniciar o `client`:
```
cd Client
sh client.sh
```
Cada terminal imprimirá as mensagens envolvidas na comunicação entre ambos.

## Teste de Timeouts

Na linha 19 do arquivo `Server/server.cpp`, e na linha 17 do arquivo `Client/client.cpp`, há um define comentado chamado `DEBUG`. 
Caso um deles seja descomentado, o servidor ou o cliente passam a responder uma mensagem apenas quando o usuário apertar a tecla ENTER.
Isso é útil para testar o tratamento de timeout por parte de ambos os lados.
É recomendável usar o debug apenas em um dos dois de cada vez, pois, caso contrário, o usuário terá que mudar de terminal continuamente para comandar a resposta das requisições.
