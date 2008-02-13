#!/bin/bash
for f in `find -name '*.pyc'`; do rm $f; done
