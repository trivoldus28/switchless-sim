/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrdered");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DimensionOrdered);

TypeId
DimensionOrdered::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::DimensionOrdered")
        .SetParent<Object> ();

    return tid;
}

DimensionOrdered::DimensionOrdered ()
{
    NS_LOG_FUNCTION (this);
}

DimensionOrdered::~DimensionOrdered ()
{
    NS_LOG_FUNCTION (this);
}

std::string
DimensionOrdered::InterfaceDirectionToAscii (InterfaceDirection dir)
{
    switch (dir)
    {
        case X_POS:
            return "X_POS";
        case X_NEG:
            return "X_NEG";
        case Y_POS:
            return "Y_POS";
        case Y_NEG:
            return "Y_NEG";
        case Z_POS:
            return "Z_POS";
        case Z_NEG:
            return "Z_NEG";
        case LOOPBACK:
            return "LOOPBACK";
        case NUM_DIRS:
            return "NUM_DIRS";
        case INVALID_DIR:
            return "INVALID_DIR";
        default:
            return "INVALID_DIR";
    }

    return "INVALID_DIR";
}

} // namespace ns3
