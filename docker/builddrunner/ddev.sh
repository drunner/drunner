#!/bin/bash
# Configuration file for ddev.

# Name of Docker Hub image we eventually build to (ignoring any branch info):
BUILDNAME="drunner/builddrunner"

# Whether or not we're a dService:
DSERVICE="no"

# Non-empty to install and update dev checkout when we build this (using the specified dRunner Service name).
DEVSERVICENAME=""
