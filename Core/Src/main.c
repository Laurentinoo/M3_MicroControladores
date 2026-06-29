/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define EMPTY  0U
#define RED    1U
#define GREEN  2U

#define MULTIPLEX_INTERVAL_MS  2U
#define WINNER_BLINK_INTERVAL_MS  500U
#define DRAW_BLINK_INTERVAL_MS    500U
#define RESET_HOLD_TIME_MS          3000U
#define CENTER_RELEASE_DEBOUNCE_MS      80U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* Matriz dos botões */

GPIO_TypeDef * const buttonRowPort[3] =
{
    GPIOA,
    GPIOA,
    GPIOA
};

const uint16_t buttonRowPin[3] =
{
    GPIO_PIN_11,
    GPIO_PIN_12,
    GPIO_PIN_15
};

GPIO_TypeDef * const buttonColumnPort[3] =
{
    GPIOB,
    GPIOB,
    GPIOB
};

const uint16_t buttonColumnPin[3] =
{
    GPIO_PIN_5,
    GPIO_PIN_4,
    GPIO_PIN_3
};

/* Indica se o botão foi pressionado */
bool buttonState[3][3] =
{
    {false, false, false},
    {false, false, false},
    {false, false, false}
};

/* Mapeamento físico das linhas */
GPIO_TypeDef * const rowPort[3] = {GPIOB, GPIOB, GPIOB};
const uint16_t rowPin[3] = {GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14};

/* Mapeamento físico das colunas */
GPIO_TypeDef * const columnPort[3] = {GPIOA, GPIOA, GPIOB};
const uint16_t columnPin[3] = {GPIO_PIN_9, GPIO_PIN_8, GPIO_PIN_15};

/* Controlam qual posição da matriz será exibida agora */
static uint8_t scanRow = 0U;
static uint8_t scanColumn = 0U;
static uint32_t lastMultiplexTime = 0U;

// Ligação dos LEDs na placa
// B12xB15 | B12xB15 | B12xB15
// B13xA08 | B13xA08 | B13xA08
// B14xA09 | B14xA09 | B14xA09
/*
uint8_t tic_tac_toe[3][3] =
{
    {EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY}
};*/

uint8_t tic_tac_toe[3][3] =
{
    {EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY}
};
// EMPTY = Apagado
// RED	 = Vermelho
// GREEN = Verde

bool Turn = false;
// False = 1/X
// True  = 2/O
uint8_t winner = EMPTY;
bool gameOver = false;
bool isDraw = false;

/* Controle do pisca quando alguém vence */
static bool winnerBlinkOn = true;
static uint32_t lastWinnerBlinkTime = 0U;

/* Controle do pisca em xadrez quando a partida termina empatada */
static bool drawPatternA = true;
static uint32_t lastDrawBlinkTime = 0U;

/* Controle do botão central [1][1] para reinício secreto */
static uint32_t centerButtonPressStart = 0U;
static bool centerButtonTiming = false;
static bool centerButtonResetLocked = false;

/* Evita uma falsa nova jogada causada pelo bounce ao soltar após o reset. */
static uint32_t centerButtonReleaseStart = 0U;
static bool centerButtonReleaseTiming = false;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

static void ScanButtons(void);
static void SetAllButtonRowsHigh(void);

static void MatrixAllOff(void);
static void MatrixConfigureOutput(GPIO_TypeDef *port, uint16_t pin);
static void MatrixShowCell(uint8_t row, uint8_t column, uint8_t color);
static void RefreshLedMatrix(void);
static void RefreshWinnerBlink(void);
static void RefreshDrawBlink(void);
static void ProcessButtons(void);

static uint8_t CheckWinner(void);
static bool CheckDraw(void);
static bool CheckSecretReset(void);
static void ResetGame(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  MatrixAllOff();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

      ScanButtons();

      /* O botão central [1][1] pressionado por 5 s reinicia a partida. */
      if (CheckSecretReset() == false)
      {
          ProcessButtons();
      }

      /* Enquanto a partida estiver ativa, verifica vitória ou empate. */
      if (gameOver == false)
      {
          winner = CheckWinner();

          /* Ao detectar vencedor, inicia o pisca da cor vencedora. */
          if (winner != EMPTY)
          {
              gameOver = true;
              isDraw = false;
              winnerBlinkOn = true;
              lastWinnerBlinkTime = HAL_GetTick();
          }
          /* Se ninguém venceu e não há mais posições vazias, é empate. */
          else if (CheckDraw() == true)
          {
              gameOver = true;
              isDraw = true;
              drawPatternA = true;
              lastDrawBlinkTime = HAL_GetTick();
          }
      }

      if (gameOver == true)
      {
          if (isDraw == true)
          {
              RefreshDrawBlink();
          }
          else
          {
              RefreshWinnerBlink();
          }
      }
      else
      {
          RefreshLedMatrix();
      }

  /* USER CODE END 3 */
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_11|GPIO_PIN_12
                          |GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB12 PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA9 PA11 PA12
                           PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_11|GPIO_PIN_12
                          |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB4 PB5 (colunas dos botões) */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

static void SetAllButtonRowsHigh(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15, GPIO_PIN_SET);
}

static void ScanButtons(void)
{
    uint8_t row;
    uint8_t column;

    for (row = 0; row < 3; row++)
    {
        /*
         * Primeiro deixa todas as linhas em HIGH.
         * Depois coloca somente a linha atual em LOW.
         */
        SetAllButtonRowsHigh();

        HAL_GPIO_WritePin(buttonRowPort[row],
                          buttonRowPin[row],
                          GPIO_PIN_RESET);

        /*
         * Lê as três colunas.
         * Se a coluna estiver LOW, existe um botão pressionado
         * entre aquela linha e aquela coluna.
         */
        for (column = 0; column < 3; column++)
        {
            if (HAL_GPIO_ReadPin(buttonColumnPort[column],
                                 buttonColumnPin[column]) == GPIO_PIN_RESET)
            {
                buttonState[row][column] = true;
            }
            else
            {
                buttonState[row][column] = false;
            }
        }
    }

    /* Ao terminar, deixa todas as linhas em HIGH novamente. */
    SetAllButtonRowsHigh();
}

static void ProcessButtons(void)
{
    uint8_t row;
    uint8_t column;

    if (gameOver == true)
    {
        return;
    }

    for (row = 0U; row < 3U; row++)
    {
        for (column = 0U; column < 3U; column++)
        {
            /*
             * Depois do reinício por 5 segundos, o botão central continua
             * bloqueado até ser solto. Isso evita iniciar uma jogada nova
             * automaticamente logo após limpar o tabuleiro.
             */
            if ((row == 1U) && (column == 1U) &&
                (centerButtonResetLocked == true))
            {
                continue;
            }

            /*
             * Uma jogada só é aceita se o botão estiver pressionado
             * e a posição correspondente ainda estiver vazia.
             */
            if ((buttonState[row][column] == true) &&
                (tic_tac_toe[row][column] == EMPTY))
            {
                if (Turn == false)
                {
                    tic_tac_toe[row][column] = RED;
                }
                else
                {
                    tic_tac_toe[row][column] = GREEN;
                }

                /* Alterna: vermelho -> verde -> vermelho... */
                Turn = !Turn;

                /* Aceita apenas uma jogada por varredura. */
                return;
            }
        }
    }
}

/*
 * Mantendo o botão central [1][1] pressionado durante 5 segundos,
 * o tabuleiro é apagado e uma partida nova é iniciada.
 * Retorna true somente no ciclo em que o reinício acontece.
 */
static bool CheckSecretReset(void)
{
    bool centerPressed = buttonState[1][1];
    uint32_t currentTime = HAL_GetTick();

    /*
     * Depois que o jogo é reiniciado, o botão central fica bloqueado.
     * O bloqueio só é liberado quando ele permanece SOLTO por 80 ms.
     * Isso elimina o bounce que poderia ser interpretado como uma nova
     * pressão e registrar uma jogada no centro logo após o reset.
     */
    if (centerButtonResetLocked == true)
    {
        if (centerPressed == true)
        {
            /* Ainda está pressionado: reinicia a contagem de soltura. */
            centerButtonReleaseTiming = false;
        }
        else
        {
            /* Começou a ficar solto: inicia a confirmação de 80 ms. */
            if (centerButtonReleaseTiming == false)
            {
                centerButtonReleaseStart = currentTime;
                centerButtonReleaseTiming = true;
            }
            else if ((currentTime - centerButtonReleaseStart) >=
                     CENTER_RELEASE_DEBOUNCE_MS)
            {
                /* Soltura estável confirmada: próxima pressão será uma jogada normal. */
                centerButtonResetLocked = false;
                centerButtonReleaseTiming = false;
            }
        }

        /*
         * Mesmo no ciclo em que destrava, retorna true para impedir
         * ProcessButtons() de registrar uma jogada usando uma leitura
         * instável do botão.
         */
        return true;
    }

    /* Botão solto antes de completar 5 s: cancela a contagem. */
    if (centerPressed == false)
    {
        centerButtonTiming = false;
        return false;
    }

    /* Marca o instante em que o botão central começou a ser pressionado. */
    if (centerButtonTiming == false)
    {
        centerButtonPressStart = currentTime;
        centerButtonTiming = true;
        return false;
    }

    /* Após 5 segundos contínuos, reinicia uma única vez. */
    if ((currentTime - centerButtonPressStart) >= RESET_HOLD_TIME_MS)
    {
        ResetGame();

        centerButtonTiming = false;
        centerButtonResetLocked = true;
        centerButtonReleaseTiming = false;

        return true;
    }

    return false;
}

static void ResetGame(void)
{
    uint8_t row;
    uint8_t column;

    for (row = 0U; row < 3U; row++)
    {
        for (column = 0U; column < 3U; column++)
        {
            tic_tac_toe[row][column] = EMPTY;
        }
    }

    /* O vermelho/X inicia todas as partidas novas. */
    Turn = false;
    winner = EMPTY;
    gameOver = false;
    isDraw = false;

    /* Restaura os estados visuais para a próxima partida. */
    winnerBlinkOn = true;
    lastWinnerBlinkTime = HAL_GetTick();
    drawPatternA = true;
    lastDrawBlinkTime = HAL_GetTick();

    /* Também limpa a confirmação de soltura do botão central. */
    centerButtonReleaseTiming = false;
    centerButtonReleaseStart = 0U;

    /* Reinicia a varredura visual pela posição [0][0]. */
    scanRow = 0U;
    scanColumn = 0U;

    MatrixAllOff();
}

static uint8_t CheckWinner(void)
{
    uint8_t i;

    /* Verifica as 3 linhas */
    for (i = 0U; i < 3U; i++)
    {
        if ((tic_tac_toe[i][0] != EMPTY) &&
            (tic_tac_toe[i][0] == tic_tac_toe[i][1]) &&
            (tic_tac_toe[i][1] == tic_tac_toe[i][2]))
        {
            return tic_tac_toe[i][0];
        }
    }

    /* Verifica as 3 colunas */
    for (i = 0U; i < 3U; i++)
    {
        if ((tic_tac_toe[0][i] != EMPTY) &&
            (tic_tac_toe[0][i] == tic_tac_toe[1][i]) &&
            (tic_tac_toe[1][i] == tic_tac_toe[2][i]))
        {
            return tic_tac_toe[0][i];
        }
    }

    /* Verifica a diagonal principal */
    if ((tic_tac_toe[0][0] != EMPTY) &&
        (tic_tac_toe[0][0] == tic_tac_toe[1][1]) &&
        (tic_tac_toe[1][1] == tic_tac_toe[2][2]))
    {
        return tic_tac_toe[0][0];
    }

    /* Verifica a diagonal secundária */
    if ((tic_tac_toe[0][2] != EMPTY) &&
        (tic_tac_toe[0][2] == tic_tac_toe[1][1]) &&
        (tic_tac_toe[1][1] == tic_tac_toe[2][0]))
    {
        return tic_tac_toe[0][2];
    }

    /* Nenhum jogador venceu */
    return EMPTY;
}

static bool CheckDraw(void)
{
    uint8_t row;
    uint8_t column;

    /* Se existir ao menos uma posição vazia, a partida ainda continua. */
    for (row = 0U; row < 3U; row++)
    {
        for (column = 0U; column < 3U; column++)
        {
            if (tic_tac_toe[row][column] == EMPTY)
            {
                return false;
            }
        }
    }

    /* Tabuleiro cheio; a função só é usada depois de CheckWinner(). */
    return true;
}

static void MatrixAllOff(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void MatrixConfigureOutput(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

static void MatrixShowCell(uint8_t row, uint8_t column, uint8_t color)
{
    GPIO_TypeDef *activeRowPort;
    GPIO_TypeDef *activeColumnPort;

    uint16_t activeRowPin;
    uint16_t activeColumnPin;

    activeRowPort = rowPort[row];
    activeRowPin = rowPin[row];

    activeColumnPort = columnPort[column];
    activeColumnPin = columnPin[column];

    MatrixAllOff();

    if (color == EMPTY) {
        return;
    }

    if (color == RED) {
        HAL_GPIO_WritePin(activeRowPort, activeRowPin, GPIO_PIN_SET);

        HAL_GPIO_WritePin(activeColumnPort, activeColumnPin, GPIO_PIN_RESET);
    } else if (color == GREEN) {
        HAL_GPIO_WritePin(activeRowPort, activeRowPin, GPIO_PIN_RESET);

        HAL_GPIO_WritePin(activeColumnPort, activeColumnPin, GPIO_PIN_SET);
    } else {
        return;
    }

    MatrixConfigureOutput(activeRowPort, activeRowPin);
    MatrixConfigureOutput(activeColumnPort, activeColumnPin);
}

static void RefreshLedMatrix(void)
{
    uint32_t currentTime = HAL_GetTick();

    /*
     * A cada 1 ms, mostra uma posição diferente.
     * São 9 posições: o tabuleiro inteiro é atualizado
     * aproximadamente a cada 9 ms.
     */
    if ((currentTime - lastMultiplexTime) < MULTIPLEX_INTERVAL_MS)
    {
        return;
    }

    lastMultiplexTime = currentTime;

    MatrixShowCell(scanRow,
                   scanColumn,
                   tic_tac_toe[scanRow][scanColumn]);

    /* Avança: [0][0], [0][1], [0][2], [1][0]... */
    scanColumn++;

    if (scanColumn >= 3U)
    {
        scanColumn = 0U;
        scanRow++;

        if (scanRow >= 3U)
        {
            scanRow = 0U;
        }
    }
}

static void RefreshWinnerBlink(void)
{
    uint32_t currentTime = HAL_GetTick();

    /* Alterna entre todos ligados e todos apagados a cada 500 ms. */
    if ((currentTime - lastWinnerBlinkTime) >= WINNER_BLINK_INTERVAL_MS)
    {
        lastWinnerBlinkTime = currentTime;
        winnerBlinkOn = !winnerBlinkOn;
    }

    /* Mantém a varredura dos 9 LEDs ativa durante o pisca. */
    if ((currentTime - lastMultiplexTime) < MULTIPLEX_INTERVAL_MS)
    {
        return;
    }

    lastMultiplexTime = currentTime;

    if (winnerBlinkOn == true)
    {
        /* Acende a posição atual na cor de quem venceu. */
        MatrixShowCell(scanRow, scanColumn, winner);
    }
    else
    {
        /* Fase apagada do pisca. */
        MatrixShowCell(scanRow, scanColumn, EMPTY);
    }

    /* Avança: [0][0], [0][1], ... [2][2]. */
    scanColumn++;

    if (scanColumn >= 3U)
    {
        scanColumn = 0U;
        scanRow++;

        if (scanRow >= 3U)
        {
            scanRow = 0U;
        }
    }
}

/*
 * Exibe um padrão em xadrez quando a partida termina sem vencedor:
 *
 * Padrão A:                  Padrão B:
 * RED   GREEN RED           GREEN RED   GREEN
 * GREEN RED   GREEN         RED   GREEN RED
 * RED   GREEN RED           GREEN RED   GREEN
 */
static void RefreshDrawBlink(void)
{
    uint32_t currentTime = HAL_GetTick();
    uint8_t color;

    /* Alterna entre os dois padrões em xadrez. */
    if ((currentTime - lastDrawBlinkTime) >= DRAW_BLINK_INTERVAL_MS)
    {
        lastDrawBlinkTime = currentTime;
        drawPatternA = !drawPatternA;
    }

    /* Mantém a multiplexação de uma célula por vez. */
    if ((currentTime - lastMultiplexTime) < MULTIPLEX_INTERVAL_MS)
    {
        return;
    }

    lastMultiplexTime = currentTime;

    /*
     * Posições cuja soma linha+coluna é par ocupam as quinas e o centro.
     * No padrão A, elas são vermelhas; no padrão B, verdes.
     */
    if (((scanRow + scanColumn) % 2U) == 0U)
    {
        color = (drawPatternA == true) ? RED : GREEN;
    }
    else
    {
        color = (drawPatternA == true) ? GREEN : RED;
    }

    MatrixShowCell(scanRow, scanColumn, color);

    /* Avança: [0][0], [0][1], ... [2][2]. */
    scanColumn++;

    if (scanColumn >= 3U)
    {
        scanColumn = 0U;
        scanRow++;

        if (scanRow >= 3U)
        {
            scanRow = 0U;
        }
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */

  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
