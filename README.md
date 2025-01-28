# Event-Driven Simulation Framework

A C++ framework for transforming iterative processes into event-driven systems using the sender/receiver pattern ([P2300](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2300r10.html)).

## Proof of Concept

**Warning:** This project is a proof of concept and it is incomplete!

## Overview

This framework allows wrapping existing iterative processes (like simulations) into event sources that can be observed and controlled asynchronously.
It provides a scheduling context similar to networking frameworks (e.g., io_context, io_uring_context) but specialized for iteration-based computations.

## Design Goals

1. **Composability**: Leverage sender/receiver pattern for seamless integration with existing asynchronous code
2. **Flexibility**: Support various observation patterns through coroutine-based agents
3. **Performance**: Zero overhead for unused features
4. **Simplicity**: Minimal requirements for adapting existing code

## Example

An example of the concept can be found in [sources/apps/example](sources/apps/example/src/main.cpp).

## Requirements

- C++26 or later
- Support for coroutines