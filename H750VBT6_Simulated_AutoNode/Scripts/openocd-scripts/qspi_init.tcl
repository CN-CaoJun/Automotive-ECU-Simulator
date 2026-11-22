proc qspi_init_pins {} {
    # GPIOB/D/E and QUADSPI clocks
    mmw 0x580244E0 0x0000001A 0x00000000

    # Reset then enable QUADSPI
    mmw 0x580244D0 0x00004000 0x00000000
    sleep 1
    mmw 0x580244D0 0x00000000 0x00004000
    mmw 0x580244D4 0x00004000 0x00000000

    # PB2 = QUADSPI_CLK/AF9, PB6 = QUADSPI_BK1_NCS/AF10
    mmw 0x58020400 0x00002020 0x00003030
    mmw 0x58020408 0x00003030 0x00003030
    mmw 0x5802040C 0x00001000 0x00003030
    mmw 0x58020420 0x0A000900 0x0F000F00

    # PD11/PD12/PD13 = QUADSPI_BK1_IO0/IO1/IO3, AF9
    mmw 0x58020C00 0x0A800000 0x0FC00000
    mmw 0x58020C08 0x0FC00000 0x0FC00000
    mmw 0x58020C0C 0x05400000 0x0FC00000
    mmw 0x58020C24 0x00999000 0x00FFF000

    # PE2 = QUADSPI_BK1_IO2, AF9
    mmw 0x58021000 0x00000020 0x00000030
    mmw 0x58021008 0x00000030 0x00000030
    mmw 0x5802100C 0x00000010 0x00000030
    mmw 0x58021020 0x00000900 0x00000F00

    # 16 MiB, CS high time 4, HSI 64 MHz / 8 = 8 MHz
    mww 0x52005000 0x00000000
    mww 0x52005004 0x00170400
    mww 0x52005000 0x07000019
    sleep 10
}

proc qspi_wait_tcf {} {
    for {set i 0} {$i < 1000} {incr i} {
        if {[mrw 0x52005008] & 0x00000002} {
            return
        }
        sleep 1
    }
    error "QSPI TCF timeout"
}

proc qspi_read_id {} {
    mww 0x5200500C 0x00000002
    mww 0x52005010 0x00000002
    mww 0x52005014 0x0500019F
    qspi_wait_tcf
    set id [mrw 0x52005020]
    echo [format "QSPI JEDEC ID: %02X %02X %02X" \
        [expr {$id & 0xFF}] \
        [expr {($id >> 8) & 0xFF}] \
        [expr {($id >> 16) & 0xFF}]]
    mww 0x5200500C 0x00000002
}

proc qspi_memory_map {} {
    # Keep single-line mode: stmqspi inherits CCR line modes for its READ_ID
    mww 0x52005014 0x0D20250B
}

proc qspi_prepare {} {
    qspi_init_pins
    qspi_read_id
    qspi_memory_map
}
