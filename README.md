
# Pacman em Assembly P3

Pacman desenvolvido por mim, em Assembly P3, na disciplina de Arquitetura de Computadores no CEFET/RJ, curso de Engenharia de Computação.


## Funcionalidades

- É possivel movimentar o pacman pelo mapa, utilizando o teclado.
- Não é possivel comer paredes com o pacman, há colisão.
- Caso o Pacman se colida com o fantasma, automaticamente voltará a sua posição inicial e o número de vidas será decrementado.
- Ao coletar pontos, o número de pontos é incrementado.
- Se o número de vidas for igual a zero, o jogo é encerrado.
- Caso todos os pontos sejam coletados, o jogador vence.
- Existem dois fantasmas, quando colidem com a parede, é gerado um movimento aleatório para que continue se movimentando pelo mapa.
## Instalação

Para instalar o pacman, é necessário possuir o git e o Java atualizado, pois o emulador do Assembly P3 é feito em java. Para clonar o projeto, crie uma pasta no local desejado.
Abra o prompt de comando e copie o diretório da pasta criada, em que será instalado o projeto. Como por exemplo:

```bash
  cd C:\Users\exemplo\Desktop\Pacman
```

Após isso, execute os seguintes comandos: 

```bash
  git init
  git clone https://github.com/P3dream/PacmanAS.git
```

Após clonar o projeto em sua máquina, vá para o seguinte diretório:

```bash
  cd C:\Users\exemplo\Desktop\Pacman\assembler\binaries
```

Selecione o sistema operacional em que deseja executa-lo, Windows, Linux ou Mac, abrindo sua respectiva pasta. 

### No Linux digite os seguintes comandos:

```bash
  chmod +x p3as-linux
  ./p3as-linux pacman.as
  java -jar p3sim.jar pacman.exe
```

### No Windows:

```bash
  p3as-win.exe pacman.as
  java -jar p3sim.jar pacman.exe
```

Após executar o simulador, será aberta a sua janela e siga os seguintes passos.
* Clique em definições
  * Clique em define IVAD e defina a tabela de interrupções na seguinte ordem:
  * __IVAD0__: D;
  * __IVAD1__: A;
  * __IVAD2__: W;
  * __IVAD3__: S; 
* Clique em __OK__
* Utilize o atalho Ctrl + R + T 

Após esses passos, basta jogar e se divertir! :)
## Autores

- [@P3dream](https://www.github.com/P3dream)


## 🔗 Links
[![linkedin](https://img.shields.io/badge/linkedin-0A66C2?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/)


