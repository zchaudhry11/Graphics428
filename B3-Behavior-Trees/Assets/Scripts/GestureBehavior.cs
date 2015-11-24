using UnityEngine;
using TreeSharpPlus;
using System.Collections.Generic;

public class GestureBehavior : MonoBehaviour 
{
    //Transform of meeting points
    public Transform destination1; 
    public Transform destination2;
    public Transform destination3;
    public Transform destination4;

    //Characters that are performing behaviors
    public GameObject participant; 
    public GameObject friend1;
    public GameObject friend2;

    private float agentDistance = 0;

    private bool _arrived1 = false; //Flag raised when friend is near destination
    private bool _arrived2 = false;
    private bool _arrived3 = false;
    private bool _arrived4 = false;

    private int zone1 = 0; //Number of characters at meeting point
    private int zone2 = 0;
    private int zone3 = 0;
    private int zone4 = 0;

    private bool _conv1 = false; //Flag raised when conversation complete
    private bool _conv2 = false;
    private bool _conv3 = false;
    private bool _conv4 = false;

    private bool _haveBall = true; //Raised when character has ball, lowered when dropped

    //Mecanim of participants
    private BehaviorMecanim charMec; 
    private BehaviorMecanim friend1Mec;
    private BehaviorMecanim friend2Mec;

    private BehaviorAgent behaviorAgent;

    private GameObject[] characters;

    void Start()
    {
        charMec = participant.GetComponent<BehaviorMecanim>();
        friend1Mec = friend1.GetComponent<BehaviorMecanim>();
        friend2Mec = friend2.GetComponent<BehaviorMecanim>();

        behaviorAgent = new BehaviorAgent(this.BuildTreeRoot());
        BehaviorManager.Instance.Register(behaviorAgent);
        behaviorAgent.StartBehavior();
    }

    void Update()
    {
        destination1 = GameObject.FindGameObjectWithTag("MP1").transform;
        zone1 = 0;
        zone2 = 0;
        zone3 = 0;
        zone4 = 0;

        //Spherecast to see if there are ever 2 or more characters at a meeting point
        RaycastHit[] hit;
        hit = Physics.SphereCastAll(destination1.position, 2f, transform.up, Mathf.Infinity);

        for (int i = 0; i < hit.Length; i++)
        {
            if (hit[i].transform.gameObject.tag == "Character")
            {
                zone1++;
            }
        }

        if (zone1 >= 2)
        {
            //print("Agents met up at zone 1!");
            _arrived1 = true;

            //print(zone1);
        }

        RaycastHit[] hit2;
        hit2 = Physics.SphereCastAll(destination2.position, 2f, transform.up, Mathf.Infinity);

        for (int i = 0; i < hit2.Length; i++)
        {
            if (hit2[i].transform.gameObject.tag == "Character")
            {
                zone2++;
            }
        }

        if (zone2 >= 2)
        {
          //  print("Agents met up at zone 2!");
            _arrived2 = true;

            //print(zone2);
        }

        RaycastHit[] hit3;
        hit3 = Physics.SphereCastAll(destination3.position, 2f, transform.up, Mathf.Infinity);

        for (int i = 0; i < hit3.Length; i++)
        {
            if (hit3[i].transform.gameObject.tag == "Character")
            {
                zone3++;
            }
        }

        if (zone3 >= 2)
        {
           // print("Agents met up at zone 3!");
            _arrived3 = true;

            //print(zone3);
        }

        RaycastHit[] hit4;
        hit4 = Physics.SphereCastAll(destination4.position, 2f, transform.up, Mathf.Infinity);

        for (int i = 0; i < hit4.Length; i++)
        {
            if (hit4[i].transform.gameObject.tag == "Character")
            {
                zone4++;
            }
        }

        if (zone4 >= 2)
        {
           // print("Agents met up at zone 4!");
            _arrived4 = true;


            //print(zone4);
        }
        
    }

    protected Node ST_Approach(Transform target)
    {
        Val<Vector3> position = Val.V(() => target.position); //Convert position to type Val so it can be passed into tree

        Val<Vector3> friendPos = Val.V(() => friend1.transform.position);

        Val<Vector3> friendPos2 = Val.V(() => friend2.transform.position);

        Val<string> yawnAnim = Val.V(() => "Yawn");

        float charDist = 2.0f;
        Val<float> radius = Val.V(() => charDist);

        float rand = Random.Range(0, 12);

        if (rand > 3 && rand < 7)
        {
            return new Sequence(
                charMec.Node_GoTo(new Vector3(target.position.x, target.position.y, target.position.z - 1)),
                charMec.Node_HandAnimation(yawnAnim, true), new LeafWait(1000), charMec.Node_HandAnimation(yawnAnim, false)
                );
        }
        else if (rand < 3)
        {
            return new Sequence(
                friend1Mec.Node_GoTo(new Vector3(target.position.x, target.position.y, target.position.z - 1)),
                friend1Mec.Node_HandAnimation(yawnAnim, true), new LeafWait(1000), friend1Mec.Node_HandAnimation(yawnAnim, false)
                );
        }
       /* else if (rand > 7 && rand < 10)
        {
            return new Sequence(
                friend2Mec.Node_GoTo(new Vector3(target.position.x, target.position.y, target.position.z - 1)),
                friend2Mec.Node_HandAnimation(yawnAnim, true), new LeafWait(1000), friend2Mec.Node_HandAnimation(yawnAnim, false)
                );
        }*/
        else if (rand >= 7)
        {
            //print("triple");
            return new Sequence(
                //charMec.Node_GoToUpToRadius((new Vector3(target.position.x, target.position.y, target.position.z - 1)), radius),
                //friend1Mec.Node_GoToUpToRadius((new Vector3(target.position.x, target.position.y, target.position.z - 1)), radius), 
                charMec.Node_GoTo(new Vector3(target.position.x, target.position.y, target.position.z - 1)),
                friend1Mec.Node_GoTo(new Vector3(target.position.x, target.position.y, target.position.z - 1)),
                //friend2Mec.Node_GoTo(new Vector3(target.position.x, target.position.y, target.position.z - 1)),
                new SequenceShuffle(
                    ST_Converse(),
                    ST_Dance()
                    )
                );
        }

        return new Sequence(charMec.Node_GoTo(new Vector3(target.position.x, target.position.y, target.position.z - 1)), new LeafWait(250));
    }

    protected Node ST_Dance()
    {
        Val<Vector3> partPos = Val.V(() => participant.transform.position);
        Val<Vector3> f1Pos = Val.V(() => friend1.transform.position);
        Val<Vector3> f2Pos = Val.V(() => friend2.transform.position);

        Val<string> danceAnim = Val.V(() => "Breakdance");
        Val<string> clapAnim = Val.V(() => "Clap");
        
        return new Sequence(
                    new Sequence(
                        charMec.Node_OrientTowards(f1Pos), friend1Mec.Node_OrientTowards(partPos)
                    ),
                    new Sequence(
                        charMec.Node_BodyAnimation(danceAnim, true), new LeafWait(5000), charMec.Node_BodyAnimation(danceAnim, false),
                        friend1Mec.Node_HandAnimation(clapAnim, true), new LeafWait(1000), friend1Mec.Node_HandAnimation(clapAnim, false),
                        friend1Mec.Node_BodyAnimation(danceAnim, true), new LeafWait(3000), friend1Mec.Node_BodyAnimation(danceAnim, false)
                       // friend2Mec.Node_HandAnimation(clapAnim, true), new LeafWait(1000), friend2Mec.Node_HandAnimation(clapAnim, false)
                        )
            );
    }

    protected Node ST_Converse()
    {
        Val<string> thinkAnim = Val.V(() => "Think");
        Val<string> surpAnim = Val.V(() => "Surprised");
        Val<string> bcAnim = Val.V(() => "BeingCocky");
        Val<string> cheerAnim = Val.V(() => "Cheer");

        Val<Vector3> partPos = Val.V(() => participant.transform.position);
        Val<Vector3> f1Pos = Val.V(() => friend1.transform.position);
        Val<Vector3> f2Pos = Val.V(() => friend2.transform.position);
        //print("Executed first!");
        /*
        GameObject[] participants = GameObject.FindGameObjectsWithTag("Character");
        List<Sequence> sequences = new List<Sequence>();
        for (int i = 0; i < participants.Length-1; i++)
        {
           if (i + 1 < participants.Length)
            {
                sequences.Add(new Sequence(
                      participants[i].GetComponent<BehaviorMecanim>().Node_OrientTowards(f1Pos), participants[i + 1].GetComponent<BehaviorMecanim>().Node_OrientTowards(partPos)
                  ));

                sequences.Add(
                new Sequence(
                    participants[i].GetComponent<BehaviorMecanim>().Node_HandAnimation(bcAnim, true), new LeafWait(1180), participants[i].GetComponent<BehaviorMecanim>().Node_HandAnimation(bcAnim, false),
                    participants[i + 1].GetComponent<BehaviorMecanim>().Node_HandAnimation(bcAnim, true), new LeafWait(1180), participants[i + 1].GetComponent<BehaviorMecanim>().Node_HandAnimation(bcAnim, false),
                    participants[i].GetComponent<BehaviorMecanim>().Node_HandAnimation(surpAnim, true), new LeafWait(4000), participants[i].GetComponent<BehaviorMecanim>().Node_HandAnimation(surpAnim, false),
                    participants[i + 1].GetComponent<BehaviorMecanim>().Node_HandAnimation(cheerAnim, true), new LeafWait(1180), participants[i + 1].GetComponent<BehaviorMecanim>().Node_HandAnimation(cheerAnim, false)
                    )
                    );
            }
        }
        */

        return new Sequence(
            new Sequence(
                charMec.Node_OrientTowards(f1Pos), friend1Mec.Node_OrientTowards(partPos) //friend2Mec.Node_OrientTowards(f1Pos)
            ),
            new Sequence(
                charMec.Node_HandAnimation(bcAnim, true), new LeafWait(1180), charMec.Node_HandAnimation(bcAnim, false),
                friend1Mec.Node_HandAnimation(bcAnim, true), new LeafWait(1180), friend1Mec.Node_HandAnimation(bcAnim, false),
              //  friend2Mec.Node_HandAnimation(thinkAnim, true), new LeafWait(1180), friend2Mec.Node_HandAnimation(thinkAnim, false),
                charMec.Node_HandAnimation(surpAnim, true), new LeafWait(4000), charMec.Node_HandAnimation(surpAnim, false)
              //  friend1Mec.Node_HandAnimation(cheerAnim, true), new LeafWait(1180), friend1Mec.Node_HandAnimation(cheerAnim, false),
               // friend2Mec.Node_HandAnimation(bcAnim, true), new LeafWait(1180), friend2Mec.Node_HandAnimation(bcAnim, false)
                ));

       /* return new SequenceParallel(charMec.Node_HandAnimation(thinkAnim, true), new LeafWait(250), charMec.Node_HandAnimation(surpAnim, true),
               new LeafWait(250), friend1Mec.Node_HandAnimation(bcAnim, true), new LeafWait(250), friend1Mec.Node_HandAnimation(cheerAnim, true)
                );
        */
    }

    private GameObject GetRightHandItem(GameObject actor)
    {
        GameObject rightHand = participant.transform.GetChild(0).transform.GetChild(0).transform.GetChild(0).transform.GetChild(0).transform.GetChild(0).transform.GetChild(1).transform.GetChild(1).transform.GetChild(0).transform.GetChild(4).transform.GetChild(0).transform.GetChild(1).transform.GetChild(2).gameObject;
        GameObject item = null;
        if (rightHand.transform.childCount >= 9)
        {
            item = rightHand.transform.GetChild(8).gameObject;
        }

        return item;
    }

    private GameObject GetRightHand(GameObject actor)
    {
        GameObject rightHand = participant.transform.GetChild(0).transform.GetChild(0).transform.GetChild(0).transform.GetChild(0).transform.GetChild(0).transform.GetChild(1).transform.GetChild(1).transform.GetChild(0).transform.GetChild(4).transform.GetChild(0).transform.GetChild(1).transform.GetChild(2).gameObject;
        return rightHand;
    }

    private Node BuildTreeRoot()
    {
        //print("build");
        return
            new DecoratorLoop(
                new Sequence( //Wandering behavior
                    new Sequence(this.ST_Approach(this.destination1)),
                    new Sequence(this.ST_Approach(this.destination2)),
                    new Sequence(this.ST_Approach(this.destination3)),
                    new Sequence(this.ST_Approach(this.destination4))
                     )
                );
        
    }
}