# ATM Simulation System

A console-based ATM simulation system written in C that mimics real ATM operations including deposit, withdrawal, balance inquiry, and transaction history.

## Features

- **User Authentication**: Secure login with account number and PIN
- **Account Management**: Deposit and withdraw funds
- **Balance Inquiry**: Check current account balance
- **Transaction History**: View all past transactions with timestamps
- **Cross-Platform**: Works on Windows and Unix/Linux systems
- **Data Persistence**: Saves account and transaction data to files

## Requirements

- C Compiler (GCC, Clang, or MSVC)
- Standard C Library

## Compilation

### Linux/Mac:
```bash
gcc -o atm atm_simulation.c
