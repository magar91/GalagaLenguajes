#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QKeyEvent"
#include "Structs.h"
#include <stdlib.h>
#include "QVBoxLayout"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->running = true;
    this->setFocus();
    setFixedSize(GAME_WIDTH,GAME_HEIGHT);
    ui->backgroundFrame->setFixedSize(GAME_WIDTH,GAME_HEIGHT);
    //inicializacion atributos

    AspectRatioPixmapLabel* labelprueba = new AspectRatioPixmapLabel();
    QPixmap pixPrueba(":/gameElement/assets/spaceship1.png");
    labelprueba->setPixmap(pixPrueba);
    labelprueba->setFixedSize(50,50);
    labelprueba->setParent(this);
    labelprueba->show();
    labelprueba->move(GAME_WIDTH/2,GAME_HEIGHT- 70);
    labelprueba->setScaledContents(true);

    player = newPlayerShip(labelprueba);
    playerBullets = new QList<PlayerBeam*>();
    enemies = new QList<enemy_T*>();
    scoreNumbers = new QList<QLabel*>();
    Lives = new QList<QLabel*>();

    //Inicializacion variables misc
    numbers[0] = ":/miscelanious/assets/numeral0.png";
    numbers[1] = ":/miscelanious/assets/numeral1.png";
    numbers[2] = ":/miscelanious/assets/numeral2.png";
    numbers[3] = ":/miscelanious/assets/numeral3.png";
    numbers[4] = ":/miscelanious/assets/numeral4.png";
    numbers[5] = ":/miscelanious/assets/numeral5.png";
    numbers[6] = ":/miscelanious/assets/numeral6.png";
    numbers[7] = ":/miscelanious/assets/numeral7.png";
    numbers[8] = ":/miscelanious/assets/numeral8.png";
    numbers[9] = ":/miscelanious/assets/numeral9.png";

    //----------------------------------

    //
    generateNextLevel();

    //Asignación de los Threads
    this->gameloop = new GameLoopThread(true);

    //Connects necesarios para el funcionamiento
    connect(gameloop,SIGNAL(gameUpdate()),this,SLOT(gameUpdate()));
    connect(gameloop,SIGNAL(gameRender()),this,SLOT(gameRender()));
    connect(gameloop,SIGNAL(gameDrawn()),this,SLOT(gameDrawn()));

    this->gameloop->start();

}

MainWindow::~MainWindow()
{
    delete ui;
}

playerShip *MainWindow::newPlayerShip(AspectRatioPixmapLabel* label)
{
    playerShip* temp = (playerShip*)malloc(sizeof(playerShip));
    temp->position = (point*)malloc(sizeof(point));
    temp->position->x = GAME_WIDTH /2;
    temp->position->y = GAME_HEIGHT - 70;
    temp->dx = temp->dy = 0;
    temp->lives = 3;
    temp->score = 0;
    temp->speed = 12;
    temp->up = temp->down = temp->left = temp->right = temp->firing = false;
    temp->width = 50;
    temp->height = 50;
    temp->sprites[0] = ":/gameElement/assets/spaceship1.png";
    temp->sprites[1] = ":/gameElement/assets/spaceship2.png";
    temp->myLabel = label;
    return temp;
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    const int codigoTecla = event->key();

        if( codigoTecla == Qt::Key_Up)
        {
           player->up = true;
            }
        if(codigoTecla == Qt::Key_Down)
        {
           player->down = true;
        }
        if(codigoTecla == Qt::Key_Left)
        {
            player->left = true;
        }
        if(codigoTecla == Qt::Key_Right)
        {
            player->right = true;
        }
        if(codigoTecla == Qt::Key_Space)
        {
            QLabel* tempLabel = new QLabel();
            tempLabel->setParent(this);
            tempLabel->show();
            playerBullets->append(newPlayerBeam(270,player->position->x + (player->width/2),player->position->y+5,tempLabel));
        }

}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    const int codigoTecla = event->key();

    if(event->type() == QEvent::KeyRelease)
    {
        if( codigoTecla == Qt::Key_Up)
        {
           player->up = false;
        }
        if(codigoTecla == Qt::Key_Down)
        {
           player->down = false;
        }
        if(codigoTecla == Qt::Key_Left)
        {
            player->left = false;
        }
        if(codigoTecla == Qt::Key_Right)
        {
            player->right = false;
        }

    }

}

void MainWindow::refreshScore()
{
    if(enemyKilled)
    {
        enemyKilled = false;
        int num = player->score;
        clearScore();

        do
        {
            int index = num%10;
            num /= 10;
            qDebug() << "index " << index << " num " << num;
            QPixmap temp(this->numbers[index]);
            QLabel* templbl = new QLabel();
            templbl->setPixmap(temp);
            templbl->setFixedSize(15,15);
            templbl->setScaledContents(true);
            templbl->setParent(this);
            scoreNumbers->append(templbl);

        }while(num != 0);

        int start = 30 + (30* scoreNumbers->size());
        for(int i = 0; i < scoreNumbers->size(); i++)
        {
            scoreNumbers->at(i)->move(start, 30);
            scoreNumbers->at(i)->show();
            qDebug() << "pos X "<<start;
            start -=30;
        }
    }
}

void MainWindow::clearScore()
{
    for(int i = 0; i < scoreNumbers->size(); i++)
    {
        delete scoreNumbers->at(i);
        scoreNumbers->removeAt(i);
        i--;
    }
}

void MainWindow::refreshLives()
{
    if(playerHitted)
    {
        playerHitted = false;
        clearLives();

        int lives = player->lives;
        int position = GAME_WIDTH - 150;

        for(int i = 0; i < lives; i++)
        {
            QLabel* label = new QLabel();
            QPixmap pix(":/miscelanious/assets/playerLife3_red.png");

            label->setParent(this);
            label->setPixmap(pix);

            label->move(position,30);
            label->show();
            position +=40;
            Lives->append(label);


        }

    }
}

void MainWindow::clearLives()
{
    for(int i = 0; i < Lives->size(); i++)
    {
        delete Lives->at(i);
        Lives->removeAt(i);
        i--;
    }
}

void MainWindow::generateNextLevel()
{
    int posicionX = 200;
    int posicionY = 50;

    enemyType_T enemigos[3][10];

    if(level%5 == 0)
    {
        for(int i = 0; i < 10; i++)
        {
            enemigos[0][i] = DISPARADOR;
        }
        for(int i = 1; i < 3; i++)
        {
            for(int j = 0; j < 10; j++)
            {
                enemigos[i][j] = NORMAL;
            }
        }
    }
    else if(level%5 == 1)
    {
        for(int i = 0; i < 10; i++)
        {
            enemigos[0][i] = DISPARADOR;
            enemigos[2][i] = NORMAL;
        }

        for(int i = 0; i < 5; i++)
        {
            enemigos[1][i] = DISPARADOR;
            enemigos[1][i+5] = NORMAL;
        }
    }
    else if(level%5 == 2)
    {
        for(int i = 0; i < 3; i++ )
        {
            for(int j = 0; j < 10; j++)
            {
                enemigos[i][j] = DISPARADOR;
            }
        }
    }
    else if(level%5== 3)
    {
        for(int i = 0; i < 10; i++)
        {
            enemigos[0][i] = REGRESADOR;
            enemigos[1][i] = DISPARADOR;
            enemigos[2][i] = NORMAL;
        }
    }
    else if(level%5 == 4)
    {
        for(int i = 0; i < 10; i++)
        {
            enemigos[2][i] = NORMAL;
            enemigos[1][i] = DISPARADOR;
        }
        for(int i = 0; i < 5; i++)
        {
            enemigos[0][i] = REGRESADOR;
            enemigos[0][i+5] = REGDIS;
        }
    }

    if(this->level%5 == 4)
    {
        posicionX +=200;
        //CODIGO PARA CREAR EL BOSS AQUI
    }

    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 10; j++)
        {
            //CODIGO PARA EL MOVIMIENTO AQUI
            QLabel* label = new QLabel();
            label->setParent(this);
            label->show();

            enemy_T* temp = newEnemy(enemigos[i][j],1,label,posicionX,posicionY);

            enemies->append(temp);
            posicionX += 60;
        }
        posicionY += 100;
        posicionX = 200;
    }
}


void MainWindow::gameUpdate()
{
    //update jugador
    updateShip(this->player);

    //update bullets
    for(int i = 0; i < playerBullets->size(); i++)
    {
        bool remove = UpdatePlayerBeam(playerBullets->at(i));

        if(remove)
        {
            delete playerBullets->at(i)->myLabel;
            playerBullets->removeAt(i);
            i--;

        }
    }
    for(int i= 0; i < enemies->size(); i++)
    {
        enemyUpdate(enemies->at(i));
    }

    //Checkeo Colisiones
    for(int i = 0; i < playerBullets->size(); i++)
    {
        PlayerBeam* bullet = playerBullets->at(i);
        QRect Rectbullet = bullet->myLabel->geometry();

        for(int j = 0; j < enemies->size(); j++)
        {
             enemy_T* actEnemy = enemies->at(j);
             QRect RectActEnemy = actEnemy->image->geometry();
             if(Rectbullet.intersects(RectActEnemy))
             {
                 hit(actEnemy);
                 delete bullet->myLabel;
                 playerBullets->removeAt(i);
                 i--;
                 break;
             }
        }

    }
    //Checkeo Muerte de Enemigos
    for(int i = 0; i < enemies->size(); i++)
    {
        if(enemies->at(i)->dead)
        {
            delete enemies->at(i)->image;
            player->score += checkEnemyValue(enemies->at(i)->type);
            enemies->removeAt(i);
            i--;
            this->enemyKilled = true;
        }
    }

    if(enemies->size() == 0)
    {
        this->level++;
        generateNextLevel();
    }

    refreshScore();
    refreshLives();

}

void MainWindow::gameDrawn()
{

}
void MainWindow::gameRender()
{
    drawShip(this->player);
    for(int i = 0; i < playerBullets->size(); i++)
    {
        DrawPlayerBeam(playerBullets->at(i));

    }
    for(int i = 0; i < enemies->size(); i++)
    {
        enemyDraw(enemies->at(i));
    }
}


