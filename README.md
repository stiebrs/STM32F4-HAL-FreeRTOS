Companion sources for series of blog posts on implementing FreeRTOS on STM32F429-Discovery board with STM HAL 

List of topics and links to individual pages is available on [the main project page](https://blog.shirtec.com/p/blog-page.html)


[Part I: Setup (blinky)](https://blog.shirtec.com/2018/05/stm32-hal-freertos-part-i-setup-blinky.html) deals with setting up development environment on Linux, as well as Windows, to get basic blinker task off the ground.


[Part II: UART](https://blog.shirtec.com/2018/05/stm32-hal-freertos-part-ii-uart.html) is about the simplest(ish) way of implementing feedback from the devboard via UART in blocking mode.


[Part III: SPI in blocking mode](https://blog.shirtec.com/2018/05/stm32-hal-freertos-part-iii-spi-blocking.html) connects to the on-board L3GD20 gyroscope and fetches its WHO_AM_I register in blocking mode.


[Part IV: IDE (Eclipse) setup](https://blog.shirtec.com/2018/05/stm32-hal-freertos-part-iv-ide-eclipse.html) briefly shows how to import Makefile project into Eclipse and how to get it to recognize all the symbols used, so that actual errors don't get swamped out by Eclipse bitching about symbols.


[Part V: SPI with DMA](https://blog.shirtec.com/2018/06/stm32-hal-freertos-part-v-spi-with-dma.html) shows how to set up DMA communications, as well as use of queue to pass a custom data structure from interrupts and code to a separate processing task.