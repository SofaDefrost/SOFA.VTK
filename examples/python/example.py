import Sofa

def createScene(root_node):
    root_node.name = "root"
    root_node.dt = 0.005
    root_node.gravity = [0, 0, -9.81]

    plugins = root_node.addChild('plugins')

    plugins.addObject('RequiredPlugin', name="SOFAVTK")

