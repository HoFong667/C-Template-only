
    // delay
    HAL_Delay(50);

    // send to computer sample, (uart3)
	char uart_buf[50];
	int uart_buf_len;
    uart_buf_len = sprintf( uart_buf, "Timer test\r\n");
    HAL_UART_Transmit( &huart3, (uint8_t*)uart_buf, uart_buf_len, 100 );

	uint16_t timer_val;
	timer_val = __HAL_TIM_GET_COUNTER(&htim14);    
    uart_buf_len = sprintf( uart_buf, "%d us  \r\n", timer_val );
	HAL_UART_Transmit( &huart3, (uint8_t*)uart_buf, uart_buf_len, 100 );
    
    // pin IO sample    
    GPIO_PinState pin_stat;
    pin_stat = HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin);
    if( pin_stat == GPIO_PIN_SET ){
	    HAL_GPIO_WritePin( GPIOB, LD1_Pin, GPIO_PIN_SET );
    }
	else HAL_GPIO_WritePin( GPIOB, LD1_Pin, GPIO_PIN_RESET );

    // compare when timer counter equal to certain number
	if( __HAL_TIM_GET_COUNTER(&htim14) > timer_val ){
		HAL_GPIO_TogglePin( GPIOB, LD1_Pin );
		timer_val = __HAL_GET_TIM_COUNTER(&htim14);
	}

    // timer14 start
    HAL_TIM_Base_Start(&htim14);

    // timer14 start IT
    HAL_TIM_Base_Start_IT(&htim14);
    void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef * htim)
    {
        if(htim == &htim14){
            HAL_GPIO_TogglePin( GPIOB, LD1_Pin );
        }
    }

    strcpy((char*)i2c_buf, "Hello!\r\n");
	HAL_UART_Transmit( &huart3, i2c_buf, strlen((char*)i2c_buf), HAL_MAX_DELAY );
	HAL_Delay( 500 );

/* USER CODE BEGIN PFP */
static const uint8_t i2c_addr = 0x48 << 1;
static const uint8_t i2c_reg = 0x00;
uint8_t i2c_buf[12];

HAL_StatusTypeDef stat_ret;
uint16_t i2c_ret_val;
float tmp_c;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if( htim == &htim7 ){
		HAL_GPIO_TogglePin( GPIOB, LD1_Pin );
		i2c_buf[0] = i2c_reg;
		stat_ret = HAL_I2C_Master_Transmit( &hi2c1, i2c_addr, i2c_buf, 1, HAL_MAX_DELAY );
		if( stat_ret != HAL_OK ){
			strcpy((char*)i2c_buf, "I2CErrTX!\r\n");
		}
		else{
			stat_ret = HAL_I2C_Master_Receive( &hi2c1, i2c_addr, i2c_buf, 2, HAL_MAX_DELAY );
			if( stat_ret != HAL_OK ){
				strcpy((char*)i2c_buf, "I2CErrRX!\r\n");
			}
			else{
				i2c_ret_val = ((uint16_t)i2c_buf[0] << 4 | i2c_buf[1] >> 4 );
				// convert to 2' complement
				if( i2c_ret_val > 0x7FF) i2c_ret_val |= 0xF000;
				// convert to float temperature value
				tmp_c = i2c_ret_val * 0.0625;
				// convert temperature to decimal format
				tmp_c *= 100;
				sprintf((char*)i2c_buf, "%u.%02u C\r\n",
						((unsigned int)tmp_c/100),
						((unsigned int)tmp_c%100));
				HAL_UART_Transmit( &huart3, i2c_buf, strlen((char*)i2c_buf), HAL_MAX_DELAY );
			}
		}
	}
}

uint8_t rx_data[10];

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if( huart == &huart3 ){
		HAL_UART_Receive_IT( &huart3, rx_data, 4 );
		sprintf((char*)i2c_buf, "UART Received %d %d %d %d!\r\n", rx_data[0], rx_data[1], rx_data[2], rx_data[3] );
		HAL_UART_Transmit( &huart3, i2c_buf, strlen((char*)i2c_buf), HAL_MAX_DELAY );
	}
}

// 25AA040A instuctions
const uint8_t EEPROM_READ = 0b00000011;
const uint8_t EEPROM_WRITE = 0b00000010;
const uint8_t EEPROM_WRDI = 0b00000100;
const uint8_t EEPROM_WREN = 0b00000110;
const uint8_t EEPROM_RDSR = 0b00000101;
const uint8_t EEPROM_WRSR = 0b00000001;
// Global flags
volatile uint8_t spi_xmit_flag = 0;
volatile uint8_t spi_recv_flag = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if( huart == &huart3 ){
		HAL_UART_Receive_IT( &huart3, rx_data, 4 );
		sprintf((char*)i2c_buf, "UART Received %d %d %d %d!\r\n", rx_data[0], rx_data[1], rx_data[2], rx_data[3] );
		HAL_UART_Transmit( &huart3, i2c_buf, strlen((char*)i2c_buf), HAL_MAX_DELAY );
	}
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if( hspi == &hspi1 ){
		spi_xmit_flag = 1;
		HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_SET );
	}
}

/* USER CODE END PFP */
	char uart_buf[50];
	int uart_buf_len;
	char spi_buf[20];

	uint8_t ee_addr;
	uint8_t ee_wip;
	uint8_t state = 0;

  // CS pin should default as high
  HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_SET ); // say LD2 is the SS pin for SPI
  // Say something
  uart_buf_len = sprintf(uart_buf, "Test SPI\r\n");
  HAL_UART_Transmit( &huart3, (uint8_t*)uart_buf, uart_buf_len, 100);
  // Enable write enable latch, allow write operation
  HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_RESET );
  HAL_SPI_Transmit(&hspi1, (uint8_t*)&EEPROM_WREN, 1, 100);
  HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_SET );
  // Read status register
  HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_RESET );
  HAL_SPI_Transmit(&hspi1, (uint8_t*)&EEPROM_RDSR, 1, 100);
  HAL_SPI_Receive(&hspi1, (uint8_t*)spi_buf, 1, 100);
  HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_SET );
  // Print out status register
  uart_buf_len = sprintf(uart_buf,
		  "Status: 0x%02x\r\n", (unsigned int)spi_buf[0]);
  HAL_UART_Transmit(&huart3, (uint8_t*)uart_buf, uart_buf_len, 100);
  // Test bytes to write to EEPROM
  spi_buf[0] = 0xAB;
  spi_buf[1] = 0xCD;
  spi_buf[2] = 0xEF;
  // Set starting address
  ee_addr = 0x05;
  // Write 3 bytes starting at the given address
  HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_RESET );
  HAL_SPI_Transmit(&hspi1, (uint8_t*)&EEPROM_WRITE, 1, 100);
  HAL_SPI_Transmit(&hspi1, (uint8_t*)&ee_addr, 1, 100);
  HAL_SPI_Transmit(&hspi1, (uint8_t*)spi_buf, 3, 100);
  HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_SET );
  // Clear buffer
  spi_buf[0] = 0;
  spi_buf[1] = 0;
  spi_buf[2] = 0;
  // Wait until WIP bit is cleared
  ee_wip = 1;
  while(ee_wip){
	  // Read status register
	  HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_RESET );
	  HAL_SPI_Transmit(&hspi1, (uint8_t*)&EEPROM_RDSR, 1, 100);
	  HAL_SPI_Receive(&hspi1, (uint8_t*)spi_buf, 1, 100);
	  HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_SET );
	  // Mask out wip bit
	  ee_wip = spi_buf[0] & 0b00000001;
  }
  // Read 3 bytes starting at the given address
  HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_RESET );
  HAL_SPI_Transmit(&hspi1, (uint8_t*)&EEPROM_WRITE, 1, 100);
  HAL_SPI_Transmit(&hspi1, (uint8_t*)&ee_addr, 1, 100);
  HAL_SPI_Receive(&hspi1, (uint8_t*)spi_buf, 3, 100);
  HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_SET );
  // Print out byte read
  uart_buf_len = sprintf( uart_buf,
		  "0x%02x 0x%02x 0x%02x\n", spi_buf[0], spi_buf[1], spi_buf[2]);
  HAL_UART_Transmit(&huart3, (uint8_t*)uart_buf, uart_buf_len, 100);
  // Read status register
  HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_RESET );
  HAL_SPI_Transmit(&hspi1, (uint8_t*)&EEPROM_RDSR, 1, 100);
  HAL_SPI_Receive(&hspi1, (uint8_t*)spi_buf, 1, 100);
  HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_SET );
  // Print out status register
  uart_buf_len = sprintf(uart_buf,
		  "Status: 0x%02x\r\n", (unsigned int)spi_buf[0]);
  HAL_UART_Transmit(&huart3, (uint8_t*)uart_buf, uart_buf_len, 100);



  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT( &htim7 );
  HAL_UART_Receive_IT( &huart3, rx_data, 4 );

  // CS pin should default as high
  HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_SET ); // say LD2 is the SS pin for SPI
  // Say something
  uart_buf_len = sprintf(uart_buf, "Test SPI\r\n");
  HAL_UART_Transmit( &huart3, (uint8_t*)uart_buf, uart_buf_len, 100);
  // Set starting address
  ee_addr = 0x05;

  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  switch(state){
	  case 0:
		  // First 2 bytes of buffer are instruction and address
		  spi_buf[0] = EEPROM_WRITE;
		  spi_buf[1] = ee_addr;
		  // Fill buffer with stuff to write to EEPROM
		  for(int i = 0; i < 10; i ++) spi_buf[2+i] = i;
		  // Enable write enable latch, allow write operations.
		  HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_RESET);
		  HAL_SPI_Transmit(&hspi1, (uint8_t*)&EEPROM_WREN, 1, 100);
		  HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_SET);
		  // Perform non-blocking write to SPI
		  HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_RESET);
		  HAL_SPI_Transmit_IT(&hspi1, (uint8_t*)spi_buf, 12);
		  // Next state
		  state += 1;
		  break;
	  case 1:
		  if( spi_xmit_flag ){
			  // Clear the xmit flag
			  spi_xmit_flag = 0;
			  // Next state
			  state += 1;
		  }
		  break;
	  // wait for WIP bit to be cleared
	  case 2:
		  // Read status register
		  HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_RESET );
		  HAL_SPI_Transmit(&hspi1, (uint8_t*)&EEPROM_RDSR, 1, 100);
		  HAL_SPI_Receive(&hspi1, (uint8_t*)spi_buf, 1, 100);
		  HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_SET );
		  // Mask out WIP bit
		  ee_wip = spi_buf[0] & 0b00000001;
		  // if WIP is cleared, go to next state
		  if( ee_wip == 0 ) state += 1;

		  break;
	  case 3:
		  // Clear SPI buffer
		  for(int i = 0; i < 12; i++ ) spi_buf[i] = 0;
		  // Read the 0 bytes back;
		  HAL_GPIO_WritePin( GPIOB, LD2_Pin, GPIO_PIN_RESET );
		  HAL_SPI_Transmit(&hspi1, (uint8_t*)&EEPROM_READ, 1, 100);
		  HAL_SPI_Transmit(&hspi1, (uint8_t*)&ee_addr, 1, 100);
		  HAL_SPI_Receive(&hspi1, (uint8_t*)spi_buf, 10, 100);
		  // Go to next state: wait for receive to finish
		  state += 1;
		  break;
	  case 4:
		  if( spi_recv_flag ){
			  // Clear flag and go to next state
			  spi_recv_flag = 0;
			  state += 1;
		  }
		  break;
	  case 5:
		  // Print out bytes
		  for(int i = 0; i < 10; i++){
			  uart_buf_len = sprintf(uart_buf,
					  "0x%02x ", (unsigned int)spi_buf[1]);
			  HAL_UART_Transmit(&huart3, (uint8_t*)uart_buf, uart_buf_len, 100);
		  }
		  uart_buf_len = sprintf(uart_buf, "\n\r" );
		  HAL_UART_Transmit(&huart3, (uint8_t*)uart_buf, uart_buf_len, 100);

		  // Wait a few seconds before re-transmitting
		  HAL_Delay(5000);
		  state = 0;
		  break;
	  default:
		  break;
	  }

    /* USER CODE BEGIN 3 */
  }
